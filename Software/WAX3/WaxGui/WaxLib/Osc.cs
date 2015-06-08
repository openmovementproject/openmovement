// Simple OSC Transmitter and Receiver
// Dan Jackson, 2010-2011

/*
    // Simple TinyOsc receiver example (by default, bundles unpacked into message events - can handle ReceiveBundle event and prevent auto-unpacking):
    OscReceiver oscReceiver = new OscReceiver(OscReceiver.DEFAULT_PORT);
    oscReceiver.ReceivedMessage += (sender, e) => { Console.WriteLine("[OSC] @{0:u} {1}", e.Timestamp, e.Message); };
    oscReceiver.Start();

    // Simple TinyOsc transmitter example (sending a single message and sending a message bundle):
    OscTransmitter oscTransmitter = new OscTransmitter("127.0.0.1:3333");
    oscTransmitter.Send(new OscMessage("/test/message", 123, 456.0f, "value"));
    oscTransmitter.Send(new OscBundle(OscBundle.TIMESTAMP_NOW, new OscMessage("/test/bundle", 1), new OscMessage("/test/bundle", 2)));
*/

// NOTE: These classes can be made a fair bit more efficient (currently quite wasteful on buffer allocation -- e.g. when composing message bundles).

using System;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.IO;
using System.Threading;
using System.Diagnostics;

namespace WaxLib
{

    #region OscExtension
    // OscExtension -- useful extension methods
    public static class OscExtension
    {
        public static void WriteOscInt(this Stream stream, int value)
        {
            stream.WriteByte((byte)((value >> 24) & 0xff));             // 32-bit big-endian integer
            stream.WriteByte((byte)((value >> 16) & 0xff));             //
            stream.WriteByte((byte)((value >> 8) & 0xff));              //
            stream.WriteByte((byte)((value >> 0) & 0xff));              //
        }

        public static void WriteOscLong(this Stream stream, long value)
        {
            for (int b = 0; b < 8; b++) 
            {
                stream.WriteByte((byte)((value >> ((8 - 1 - b) << 3)) & 0xff));     // 64-bit big-endian integer
            }
        }

        public static void WriteOscFloat(this Stream stream, float value)
        {
            byte[] bytes = System.BitConverter.GetBytes(value);
            if (bytes.Length != 4) { throw new Exception("Float not 4 bytes."); }   // 32-bit big-endian float
            if (System.BitConverter.IsLittleEndian) { Array.Reverse(bytes); }       // LATER: Confirm floats get swizzled as well as ints between BE/LE machines
            for (int b = 0; b < bytes.Length; b++) { stream.WriteByte(bytes[b]); }
        }

        public static void WriteOscDouble(this Stream stream, double value)
        {
            byte[] bytes = System.BitConverter.GetBytes(value);
            if (bytes.Length != 8) { throw new Exception("Double not 8 bytes."); }  // 64-bit big-endian double
            if (System.BitConverter.IsLittleEndian) { Array.Reverse(bytes); }       // LATER: Confirm floats get swizzled as well as ints between BE/LE machines
            for (int b = 0; b < bytes.Length; b++) { stream.WriteByte(bytes[b]); }
        }

        public static void WriteOscString(this Stream stream, string value)
        {
            byte[] bytes = Encoding.ASCII.GetBytes(value);
            stream.Write(bytes, 0, bytes.Length);                       // string ASCII bytes
            stream.WriteByte(0);                                        // null-terminated
            for (int n = 1 + bytes.Length; n % 4 != 0; n++) { stream.WriteByte(0); }   // pad to 4-byte boundary
        }

        public static void WriteOscBlob(this Stream stream, byte[] value)
        {
            WriteOscInt(stream, value.Length);
            stream.Write(value, 0, value.Length);                       // byte[]
            for (int n = 4 + value.Length; n % 4 != 0; n++) { stream.WriteByte(0); }   // pad to 4-byte boundary
        }

        public static long ReadOscLong(this Stream stream)
        {
            ulong timestamp = 0;
            for (int b = 0; b < 8; b++) { timestamp |= (ulong)stream.ReadByte() << ((8 - 1 - b) << 3); }
            return (long)timestamp;
        }

        public static int ReadOscInt(this Stream stream)
        {
            int first = stream.ReadByte();
            if (first < 0) { return -1; }
            uint value = 0;
            value |= (uint)first << 24;
            value |= (uint)stream.ReadByte() << 16;
            value |= (uint)stream.ReadByte() << 8;
            value |= (uint)stream.ReadByte() << 0;
            return (int)value;
        }

        public static float ReadOscFloat(this Stream stream)
        {
            byte[] bytes = new byte[4];
            for (int b = 0; b < bytes.Length; b++) { bytes[b] = (byte)stream.ReadByte(); }
            if (System.BitConverter.IsLittleEndian) { Array.Reverse(bytes); }       // LATER: Confirm floats get swizzled as well as ints between BE/LE machines
            return System.BitConverter.ToSingle(bytes, 0);
        }

        public static double ReadOscDouble(this Stream stream)
        {
            byte[] bytes = new byte[8];
            for (int b = 0; b < bytes.Length; b++) { bytes[b] = (byte)stream.ReadByte(); }
            if (System.BitConverter.IsLittleEndian) { Array.Reverse(bytes); }       // LATER: Confirm floats get swizzled as well as ints between BE/LE machines
            return System.BitConverter.ToDouble(bytes, 0);
        }

        public static string ReadOscString(this Stream stream)
        {
            List<byte> bytes = new List<byte>();
            while (true)
            {
                int c = stream.ReadByte();
                if (c == -1) { if (bytes.Count != 0) { throw new Exception("EOF in string."); } else { return null; } }
                if (c == 0) { break; }  // null-terminated
                bytes.Add((byte)c);
            }
            for (int n = 1 + bytes.Count; n % 4 != 0; n++) { if (stream.ReadByte() != 0) { throw new Exception("Unexpected padding byte."); } }   // pad to 4-byte boundary
            return Encoding.ASCII.GetString(bytes.ToArray());
        }

        public static byte[] ReadOscBlob(this Stream stream)
        {
            int length = ReadOscInt(stream);
            if (length < 0) { throw new Exception("BLOB length error."); }
            byte[] value = new byte[length];
            int offset = 0;
            while (offset < length)
            {
                int read = stream.Read(value, offset, length - offset);
                if (read <= 0) { throw new Exception("EOF in BLOB."); }
                offset += read;
            }
            for (int n = 4 + length; n % 4 != 0; n++) { if (stream.ReadByte() != 0) { throw new Exception("Unexpected padding byte."); } }   // pad to 4-byte boundary
            return value;
        }

    }
    #endregion

    #region OscData
    // OscData - parent of OscMessage and OscBundle
    public abstract class OscData
    {
        // Timestamp "now" time
        public const ulong TIMESTAMP_NOW = 1UL;

        // Allows access to the raw byte buffer
        public byte[] Buffer { get; protected set; }

        // Convert an RFC-1305 NTP Timestamp into a DateTime (upper 32 bits is the number of seconds since midnight on January 1, 1900, lower 32 bits specify the fractional part of a second)
        public static DateTime DateTimeFromTimestamp(ulong timestamp)
        {
            if (timestamp == TIMESTAMP_NOW) { return DateTime.UtcNow; }
            ulong ticks = ((timestamp >> 32) * (ulong)TimeSpan.TicksPerMillisecond) + ((timestamp & 0xffffffffU) * (ulong)TimeSpan.TicksPerMillisecond / 0x100000000UL);
            return new DateTime(1900, 1, 1) + TimeSpan.FromTicks((long)ticks);
        }

        // Convert a DateTime into an RFC-1305 NTP Timestamp (upper 32 bits is the number of seconds since midnight on January 1, 1900, lower 32 bits specify the fractional part of a second)
        public static ulong TimestampFromDateTime(DateTime dateTime)
        {
            if (dateTime.Ticks == 0) { return OscData.TIMESTAMP_NOW; }
            ulong ticks = (ulong)(dateTime - new DateTime(1900, 1, 1)).Ticks;
            ulong timestamp = (ticks / (ulong)TimeSpan.TicksPerMillisecond) << 32;                                              // whole milliseconds
            timestamp |= (ticks % (ulong)TimeSpan.TicksPerMillisecond) * 0x100000000UL / (ulong)TimeSpan.TicksPerMillisecond;   // fractional milliseconds
            return timestamp;
        }

        // Static factory method - creates an instance of the OscMessage or OscBundle class
        public static OscData FromByteArray(byte[] buffer)
        {
            //Console.WriteLine(HexDump(buffer));     // Debug dump raw OSC packet

            // Check parameters are allowed
            if (buffer == null) { throw new ArgumentNullException("buffer", "OSC buffer must not be null."); }
            if (buffer.Length < 4) { throw new ArgumentException("OSC buffer too small to be valid.", "buffer"); }
            if (buffer[0] == OscMessage.ADDRESS_PREFIX)
            {
                return new OscMessage(buffer);
            }
            else if (buffer[0] == OscBundle.BUNDLE_ADDRESS[0])
            {
                return new OscBundle(buffer);
            }
            else
            {
                throw new ArgumentException("OSC buffer doesn't appear to be an address or bundle.", "buffer");
            }
        }

        // ToString
        public override string ToString()
        {
            return HexDump(Buffer);
        }

        // HexDump - Debugging call
        internal static string HexDump(byte[] buffer)
        {
            if (buffer == null) return "<null>";
            StringBuilder sb = new StringBuilder();
            for (int line = 0; line < (buffer.Length + 15) / 16; line++)
            {
                sb.AppendFormat("{0:x4}", line * 16).Append(": ");
                for (int o = 0; o < 16; o++)
                {
                    if (line * 16 + o < buffer.Length)
                    {
                        byte b = buffer[line * 16 + o];
                        char c0 = (char)((b & 0xf) < 0xa ? ('0' + (b & 0xf)) : ('a' + (b & 0xf) - 10));
                        char c1 = (char)(((b >> 4) & 0xf) < 0xa ? ('0' + ((b >> 4) & 0xf)) : ('a' + ((b >> 4) & 0xf) - 10));
                        sb.Append(c1).Append(c0).Append(' ');
                    }
                    else
                    {
                        sb.Append("   ");
                    }
                    if (o % 4 == 3) { sb.Append(' '); }
                }
                for (int o = 0; o < 16; o++)
                {
                    if (line * 16 + o < buffer.Length)
                    {
                        byte b = buffer[line * 16 + o];
                        if (b >= 32 && b <= 127) { sb.Append((char)b); }
                        else { sb.Append('.'); }
                    }
                    else
                    {
                        sb.Append(' ');
                    }
                }
                sb.Append("\r\n");
            }
            return sb.ToString();
        }

    }
    #endregion


    #region OscMessage
    // OscMessage - single OSC message
    public class OscMessage : OscData
    {
        // Addresses
        public static readonly char[] ADDRESS_DISALLOWED_CHARS = new char[] { ' ', '#', '*', ',', '?', '[', ']', '{', '}' };
        public const char ADDRESS_DELIMITER = '/';
        public const char ADDRESS_PREFIX = ADDRESS_DELIMITER;
        public const char TYPE_PREFIX = ',';

        // Standard types from OSC 1.0
        public const char TYPE_INT = 'i';           // [4] signed two's complement int32 (big-endian)
        public const char TYPE_FLOAT = 'f';         // [4] IEEE 32-bit float (big-endian)
        public const char TYPE_STRING = 's';        // [+] ASCII null-terminated string
        public const char TYPE_BLOB = 'b';          // [*] blob (byte[]) with size

        // Standard types from OSC 1.1
        public const char TYPE_TRUE = 'T';          // [0] (no data) boolean true
        public const char TYPE_FALSE = 'F';         // [0] (no data) boolean false
        public const char TYPE_NULL = 'N';          // [0] (no data) null
        public const char TYPE_IMPULSE = 'I';       // [0] (no data) impulse/bang
        public const char TYPE_TIME = 't';          // [8] 64-bit time tag (RFC-1305 NTP Timestamp Format - upper 32 bits is the number of seconds since midnight on January 1, 1900, lower 32 bits specify the fractional part of a second)

        // Non-standard/optional types (will consume only, won't produce these types - arrays ignored for now)
        public const char TYPE_SYMBOL = 'S';        // [+] Alternate type represented as an OSC-string (for example, for systems that differentiate "symbols" from "strings")
        public const char TYPE_LONG = 'h';          // [8] 64 bit big-endian two's complement integer
        public const char TYPE_DOUBLE = 'd';        // [8] 64 bit ("double") IEEE 754 floating point number
        public const char TYPE_CHAR = 'c';          // [4] an ascii character, sent as 32 bits
        public const char TYPE_RGBA = 'r';          // [4] 32 bit RGBA color
        public const char TYPE_MIDI = 'm';          // [4] 4 byte MIDI message. Bytes from MSB to LSB are: port id, status byte, data1, data2
        public const char TYPE_ARRAY_OPEN = '[';    // [0] Indicates the beginning of an array. The tags following are for data in the Array until a close brace tag is reached.
        public const char TYPE_ARRAY_CLOSE = ']';   // [0] Indicates the end of an array.

        // Address
        public string Address { get; protected set; }

        // Parameters
        public object[] Arguments { get; protected set; }


        // Create an OSC message object for the specified binary buffer
        public OscMessage(byte[] buffer)
        {
            string address;
            List<object> arguments = new List<object>();

            if (buffer == null) { throw new ArgumentNullException("buffer", "Buffer must not be null."); }
            using (MemoryStream stream = new MemoryStream(buffer))
            {
                address = stream.ReadOscString();
                if (address == null) { throw new ArgumentException("Missing address.", "buffer"); }
                if (address[0] != OscMessage.ADDRESS_PREFIX) { throw new ArgumentException("Invalid address.", "buffer"); }

                string typeTagString = stream.ReadOscString();
                if (typeTagString.Length == 0 || typeTagString[0] != TYPE_PREFIX)
                {
                    throw new ArgumentException("ERROR: This implementation requires the OSC type-tag string.", "buffer");
                }
                typeTagString = typeTagString.Substring(1);   // Remove prefix

                for (int i = 0; i < typeTagString.Length; i++)
                {
                    switch (typeTagString[i])
                    {
                        // OSC 1.0 types
                        case OscMessage.TYPE_INT:     arguments.Add(stream.ReadOscInt()); break;
                        case OscMessage.TYPE_FLOAT:   arguments.Add(stream.ReadOscFloat()); break;
                        case OscMessage.TYPE_STRING:  arguments.Add(stream.ReadOscString()); break;
                        case OscMessage.TYPE_BLOB:    arguments.Add(stream.ReadOscBlob()); break;
                        // OSC 1.1 types
                        case OscMessage.TYPE_TRUE:    arguments.Add((Boolean)true); break;
                        case OscMessage.TYPE_FALSE:   arguments.Add((Boolean)false); break;
                        case OscMessage.TYPE_NULL:    arguments.Add(null); break;
                        case OscMessage.TYPE_IMPULSE: arguments.Add(typeof(Object)); break; // Not really sure how to represent an 'impulse' object
                        case OscMessage.TYPE_TIME:    arguments.Add(OscData.DateTimeFromTimestamp((ulong)stream.ReadOscLong())); break;
                        // Non-standard/optional types
                        case OscMessage.TYPE_SYMBOL:  arguments.Add(stream.ReadOscString()); break;
                        case OscMessage.TYPE_LONG:    arguments.Add(stream.ReadOscLong()); break;
                        case OscMessage.TYPE_DOUBLE:  arguments.Add(stream.ReadOscDouble()); break;
                        case OscMessage.TYPE_CHAR:    arguments.Add((char)stream.ReadOscInt()); break;
                        case OscMessage.TYPE_RGBA:    arguments.Add(stream.ReadOscInt()); break;
                        case OscMessage.TYPE_MIDI:    arguments.Add(stream.ReadOscInt()); break;
                        case OscMessage.TYPE_ARRAY_OPEN: break;     // ignore for now
                        case OscMessage.TYPE_ARRAY_CLOSE: break;    // ignore for now
                        default: throw new ArgumentException("Unknown OSC type tag '" + typeTagString[i] + "'", "buffer");
                    }
                }
            }

            this.Buffer = buffer;
            this.Address = address;
            this.Arguments = arguments.ToArray();
        }


        // Create a binary OSC message for the specified address from the specified parameter array (allows 'params' variable number of arguments)
        public OscMessage(string address, params object[] arguments) : this(address, (IEnumerable<object>)arguments) { }

        // Create a binary OSC message for the specified address from the specified parameters
        public OscMessage(string address, IEnumerable<object> arguments)
        {
            // Check parameters are valid
            if (address == null) { throw new ArgumentNullException("address", "Address must not be null."); }
            if (!address.StartsWith(OscMessage.ADDRESS_PREFIX.ToString())) { throw new ArgumentException("Address must begin with a forward-slash ('/').", "address"); }
            if (address.IndexOfAny(OscMessage.ADDRESS_DISALLOWED_CHARS) >= 0) { throw new ArgumentException("Address contains one or more invalid characters.", "address"); }
            if (arguments == null) { throw new ArgumentNullException("arguments", "Arguments array must not be null."); }

            // Store address and arguments
            this.Address = address;
            int numArguments = 0;
            foreach (object o in arguments) { numArguments++; }
            this.Arguments = new object[numArguments];
            numArguments = 0;
            foreach (object o in arguments) { this.Arguments[numArguments++] = o; }

            // Memory stream for OSC message
            using (MemoryStream stream = new MemoryStream())
            {
                // Write address
                stream.WriteOscString(address);

                // Write param type tag:    ",iiii\0" (i = 32-bit integer)
                StringBuilder typeTag = new StringBuilder();
                typeTag.Append(',');                                  // comma indicates start of parameter type tag list
                foreach (object o in Arguments)
                {
                    if (o is SByte || o is Byte || o is Int16 || o is UInt16 || o is Int32 || o is UInt32) { typeTag.Append(OscMessage.TYPE_INT); } // signed two's complement int32 (big-endian)
                    else if (o is Int64 || o is UInt64) { typeTag.Append(OscMessage.TYPE_LONG); }   // (non-standard long int-64 type)
                    else if (o is Single) { typeTag.Append(OscMessage.TYPE_FLOAT); }                // IEEE 32-bit float (big-endian)
                    else if (o is Double) { typeTag.Append(OscMessage.TYPE_DOUBLE); }               // (non-standard double type)
                    else if (o is Char) { typeTag.Append(OscMessage.TYPE_CHAR); }                   // (non-standard char type)
                    else if (o is String) { typeTag.Append(OscMessage.TYPE_STRING); }               // ASCII null-terminated string
                    else if (o is byte[]) { typeTag.Append(OscMessage.TYPE_BLOB); }                 // blob (byte[]) with size
                    else if (o is Boolean) { typeTag.Append((bool)o ? OscMessage.TYPE_TRUE : OscMessage.TYPE_FALSE); }  // true or false type
                    else if (o == null) { typeTag.Append(OscMessage.TYPE_NULL); }                   // null
                    else if (o is DateTime) { typeTag.Append(OscMessage.TYPE_TIME); }               // time tag (RFC-1305 NTP Timestamp format - upper 32 bits is the number of seconds since midnight on January 1, 1900, lower 32 bits specify the fractional part of a second)
                    else if (o.Equals(typeof(Object))) { typeTag.Append(OscMessage.TYPE_IMPULSE); } // impulse (not sure of a good .NET representation for this)
                    else if (o is Array) { throw new ArgumentException("Arrays not yet supported.", "arguments"); }
                    else { throw new ArgumentException("One or more arguments are of an unhandled type.", "arguments"); }
                }
                stream.WriteOscString(typeTag.ToString());

                // Values           (all values are big-endian and will be padded to 4 bytes)
                foreach (object o in Arguments)
                {
                    if      (o is SByte)  { stream.WriteOscInt((int)(SByte)o); }
                    else if (o is Byte)   { stream.WriteOscInt((int)(Byte)o); }
                    else if (o is Int16)  { stream.WriteOscInt((int)(Int16)o); }
                    else if (o is UInt16) { stream.WriteOscInt((int)(UInt16)o); }
                    else if (o is Int32)  { stream.WriteOscInt((int)(Int32)o); }
                    else if (o is UInt32) { stream.WriteOscInt((int)(UInt32)o); }
                    else if (o is Int64)  { stream.WriteOscLong((long)(Int64)o); }      // (non-standard long int-64 type)
                    else if (o is UInt64) { stream.WriteOscLong((long)(UInt64)o); }     // (non-standard long int-64 type)
                    else if (o is Single) { stream.WriteOscFloat((float)(Single)o); }
                    else if (o is Double) { stream.WriteOscDouble((float)(Double)o); }  // (non-standard double type)
                    else if (o is Char)   { stream.WriteOscInt((int)(Char)o); }         // (non-standard char type, written as int-32)
                    else if (o is String) { stream.WriteOscString((String)o); }
                    else if (o is byte[]) { stream.WriteOscBlob((byte[])o); }
                    else if (o is Boolean) { }                                          // true or false type -- neither has data
                    else if (o == null) { }                                             // null type -- no data
                    else if (o is DateTime) { stream.WriteOscLong((long)OscData.TimestampFromDateTime((DateTime)o)); }
                    else if (o.Equals(typeof(Object))) { }                              // impulse (not sure of a good .NET representation for this)
                    else if (o is Array) { throw new ArgumentException("Arrays not yet supported.", "arguments"); }
                    else { throw new ArgumentException("One or more arguments are of an unhandled type.", "arguments"); }
                }

                // Return message bytes
                Buffer = stream.ToArray();
            }
        }

        // ToString
        public override string ToString()
        {
            StringBuilder sb = new StringBuilder();
            sb.Append(Address);
            foreach (object o in Arguments)
            {
                sb.Append(",");
                sb.Append(o.ToString());
            }
            return sb.ToString();
        }


    }
    #endregion


    #region OscBundle
    // OscBundle - OSC message bundle
    public class OscBundle : OscData
    {
        // Bundle address
        public const string BUNDLE_ADDRESS = "#bundle";

        // Timestamp
        public ulong Timestamp { get; protected set; }

        // Parts
        public OscData[] Parts { get; protected set; }

        // Address of first part
        public string FirstAddress
        {
            get
            {
                if (Parts.Length > 0 && Parts[0] is OscMessage) { return ((OscMessage)Parts[0]).Address; }
                return null;
            }
        }

        // Create an OSC bundle object for the specified binary buffer
        public OscBundle(byte[] buffer)
        {
            string tag;
            ulong timestamp = OscData.TIMESTAMP_NOW;
            List<OscData> parts = new List<OscData>();

            if (buffer == null) { throw new ArgumentNullException("buffer", "Buffer must not be null."); }
            using (MemoryStream stream = new MemoryStream(buffer))
            {
                tag = stream.ReadOscString();
                if (tag != OscBundle.BUNDLE_ADDRESS) { throw new ArgumentException("Unrecognized bundle type.", "buffer"); }
                timestamp = (ulong)stream.ReadOscLong();

                while (stream.Position < stream.Length)
                {
                    int length = stream.ReadOscInt();
                    if (length < 0) { throw new Exception("Bundle part has invalid length (<0)."); }
                    if (length > buffer.Length - stream.Position) { throw new Exception("Bundle part has invalid length."); }
                    byte[] partBytes = new byte[length];
                    int offset = 0;
                    while (offset < length)
                    {
                        int read = stream.Read(partBytes, offset, length - offset);
                        if (read <= 0) { throw new Exception("EOF in bundle part."); }
                        offset += read;
                    }
                    OscData part = OscData.FromByteArray(partBytes);
                    if (part == null) { break; }
                    parts.Add(part);
                }
            }

            this.Buffer = buffer;
            this.Timestamp = timestamp;
            this.Parts = parts.ToArray();
        }

        // Create a binary OSC bundle from the specified parts array (allows 'params' variable number of arguments)
        public OscBundle(ulong timestamp, params OscData[] parts) : this(timestamp, (IEnumerable<OscData>)parts) { }

        // Create a binary OSC bundle from the specified parts
        public OscBundle(ulong timestamp, IEnumerable<OscData> parts)
        {
            // Check parameters are valid
            if (parts == null) { throw new ArgumentNullException("parts", "Parts must not be null."); }

            // Store parameters
            this.Timestamp = timestamp;
            int numParts = 0;
            foreach (OscData part in parts) { numParts++; }
            this.Parts = new OscData[numParts];
            numParts = 0;
            foreach (OscData part in parts) { this.Parts[numParts++] = part; }

            // Memory stream for OSC bundle
            using (MemoryStream stream = new MemoryStream())
            {
                // An OSC Bundle consists of the OSC-string "#bundle" (null-terminated)
                stream.WriteOscString(OscBundle.BUNDLE_ADDRESS);

                // ...followed by an OSC Time Tag (The OSC-timetag is a 64-bit fixed point time tag, high 32 bits are seconds since 1/1/1900, low 32 bits are fractions of second)
                stream.WriteOscLong((long)this.Timestamp);

                // ...followed by zero or more OSC Bundle Elements. 
                foreach (OscData part in Parts)
                {
                    // Check part is valid
                    if (part == null) { throw new ArgumentNullException("parts", "None of the parts may be null."); }
                    if (part.Buffer == null) { throw new ArgumentException("None of the parts may have null data.", "parts"); }
                    if (part.Buffer.Length % 4 != 0) { throw new ArgumentException("Every part must have a length that is a multiple of 4.", "parts"); }

                    // An OSC Bundle Element consists of its size (b-e int-32, multiple of 4)...
                    stream.WriteOscInt(part.Buffer.Length);

                    // ...and its contents
                    stream.Write(part.Buffer, 0, part.Buffer.Length);
                }

                // Return bundle bytes
                Buffer = stream.ToArray();
            }
        }

        // ToString
        public override string ToString()
        {
            StringBuilder sb = new StringBuilder();
            sb.Append(BUNDLE_ADDRESS);

            sb.Append(":").AppendFormat("{0}", Timestamp).Append(":[\r\n");
            foreach (OscData part in Parts)
            {
                sb.Append(",");
                sb.Append(part.ToString());
                sb.Append("\r\n");
            }
            sb.Append("]");
            return sb.ToString();
        }

    }
    #endregion


    #region OscTransmitter
    public class OscTransmitter
    {
        // Constants
        public const string DEFAULT_RECEIVER = "127.0.0.1";
        public const int DEFAULT_PORT = 3333;       // default for TUIO

        // Private instance variables
        private IPEndPoint endPoint;
        private Socket socket;

        // Default constructor -- localhost and default port
        public OscTransmitter() : this(DEFAULT_RECEIVER, DEFAULT_PORT) { ; }

        // Receiver specified, default port unless overridden with "address:port" in address
        public OscTransmitter(string receiver) : this(receiver, DEFAULT_PORT) { ; }

        // Receiver and port specified, port can be overridden with "address:port" in address
        public OscTransmitter(string receiver, int port)
        {
            // If a port is specified in the address, override the given port
            int index = receiver.LastIndexOf(':');
            if (index >= 0)
            {
                port = int.Parse(receiver.Substring(index + 1));
                receiver = receiver.Substring(0, index);
            }

            // Use address (localhost if none specified)
            IPAddress address = null;

            if (receiver.Length > 0 && char.IsDigit(receiver[0]))
            {
                address = IPAddress.Parse(receiver);
            }

            if (address == null)
            {
                // Lookup host
                IPHostEntry ipEntry = (receiver != null && receiver.Length > 0) ? Dns.GetHostEntry(receiver) : null;

                // If receiver was specified but not found
                if (ipEntry != null && ipEntry.AddressList.Length <= 0)
                {
                    throw new Exception("Host not found ('" + receiver + "')");
                }

                address = (ipEntry != null && ipEntry.AddressList.Length > 0) ? ipEntry.AddressList[0] : IPAddress.Loopback;
            }

            // Create end-point and USP socket
            endPoint = new IPEndPoint(address, port);
            socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
        }


        // Send an OSC message or bundle
        public bool Send(OscData messageOrBundle)
        {
            // Usage: Send(new OscMessage("/address", arg1, arg2, arg3));
            // Usage: Send(new OscBundle(OscBundle.TIMESTAMP_NOW, element1, element2, element3));

            // Check parameters are valid
            if (messageOrBundle == null) { throw new ArgumentNullException("messageOrBundle", "Message or bundle parameter is null."); }
            if (messageOrBundle.Buffer == null) { throw new ArgumentException("Message or bundle has null data.", "messageOrBundle"); }
            if (messageOrBundle.Buffer.Length == 0) { throw new ArgumentException("Message or bundle has zero-length data.", "messageOrBundle"); }

            // Send UDP packet
            int sent = socket.SendTo(messageOrBundle.Buffer, endPoint);

            // Return true if successful
            return (sent == messageOrBundle.Buffer.Length);
        }

    }
    #endregion


    #region OscReceiver

    // A delegate type for received packets
    public delegate void OscBundleEventHandler(object sender, OscBundleEventArgs e);
    public delegate void OscMessageEventHandler(object sender, OscMessageEventArgs e);

    // OscEventArgs
    public abstract class OscEventArgs : EventArgs
    {
        public DateTime Timestamp { get; protected set; }
    }

    // OscBundleEventArgs
    public class OscBundleEventArgs : OscEventArgs
    {
        public OscBundleEventArgs(DateTime timestamp, OscBundle oscBundle) { Timestamp = timestamp;  Bundle = oscBundle; AutoUnpack = true; }
        public OscBundle Bundle { get; private set; }
        public bool AutoUnpack { get; set; }
    }

    // OscMessageEventArgs
    public class OscMessageEventArgs : OscEventArgs
    {
        public OscMessageEventArgs(DateTime timestamp, OscMessage oscMessage) { Timestamp = timestamp; Message = oscMessage; }
        public OscMessage Message { get; protected set; }
    }

    // Simple OSC receiver class
    public class OscReceiver : IDisposable
    {
        /*
            OscReceiver receiver = new OscReceiver();
            receiver.Received += (sender, e) =>
            {
                Console.WriteLine("[" + e.Timestamp + "] " + e.Message.ToString());
            };
            receiver.Start();
        */

        // Constants
        public const string DEFAULT_INTERFACE = null;  // all interfaces
        public const int DEFAULT_PORT = 3333;       // default for TUIO

        // Private instance variables
        private string address;
        private int port;
        private IPEndPoint endPoint;
        private UdpClient udpClient;

        // Default constructor -- default interface and default port
        public OscReceiver() : this(DEFAULT_INTERFACE, DEFAULT_PORT) { ; }

        // Receiver with specified port
        public OscReceiver(int port) : this(DEFAULT_INTERFACE, port) { ; }

        // Receiver and port specified, port can be overridden with "address:port" in address
        public OscReceiver(string address, int port)
        {
            this.address = address;
            this.port = port;
        }


        // An event that clients can use to be notified whenever a bundle/message is received
        public event OscBundleEventHandler ReceivedBundle;
        public event OscMessageEventHandler ReceivedMessage;

        // Invoke the ReceivedBundle event
        protected virtual void OnReceivedBundle(OscBundleEventArgs e)
        {
            if (ReceivedBundle != null) { ReceivedBundle(this, e); }
            if (e.AutoUnpack)
            {
                foreach (OscData part in e.Bundle.Parts)
                {
                    if (part is OscMessage)
                    {
                        OnReceivedMessage(new OscMessageEventArgs(e.Timestamp, (OscMessage)part));
                    }
                    else if (part is OscBundle)
                    {
                        OnReceivedBundle(new OscBundleEventArgs(e.Timestamp, (OscBundle)part));
                    }
                }
            }
        }

        // Invoke the ReceivedMessage event
        protected virtual void OnReceivedMessage(OscMessageEventArgs e)
        {
            if (ReceivedMessage != null) { ReceivedMessage(this, e); }
        }

        public bool Listening { get { return (thread != null); } }


        // Receiver thread
        private Thread thread = null;
        private volatile bool quitReceiver = false;

        // Destructor
        ~OscReceiver()
        {
            Stop();
        }


        // Start receiving
        public void Start()
        {
            // Ensure stopped
            Stop();


            // If a port is specified in the address, override the given port
            int index = (address == null) ? -1 : address.LastIndexOf(':');
            if (index >= 0)
            {
                port = int.Parse(address.Substring(index + 1));
                address = address.Substring(0, index);
            }

            // Lookup host
            IPHostEntry ipEntry = (address != null && address.Length > 0) ? Dns.GetHostEntry(address) : null;

            // If receiver was specified but not found
            if (ipEntry != null && ipEntry.AddressList.Length <= 0)
            {
                //Trace.TraceWarning("Host not found: " + address + " - cannot begin receiving.");
                throw new SocketException(11001);       // WSAHOST_NOT_FOUND
            }

            // Use address (any if none specified)
            IPAddress ipAddress = (ipEntry != null && ipEntry.AddressList.Length > 0) ? ipEntry.AddressList[0] : IPAddress.Any;

            // Create end-point and USP socket
            endPoint = new IPEndPoint(ipAddress, port);
            try
            {
                udpClient = new UdpClient(endPoint);
            }
            catch (SocketException e)
            {
                //Trace.TraceWarning("SocketException: " + e.Message + " - cannot begin receiving (check port not in use).");
                throw e;
            }


            // Create and start receiver thread
            quitReceiver = false;
            thread = new Thread(new ThreadStart(this.Listener));
            thread.Start();
        }

        public void Join()
        {
            if (thread != null)
            {
                thread.Join();
            }
        }

        // Stop receiving
        public void Stop()
        {
            // Stop listening thread if exists
            if (thread != null)
            {
                // Set quit flag, interrupt thread and wait to terminate...
                quitReceiver = true;
                thread.Interrupt();
                if (!thread.Join(250))
                {
                    // If that hasn't worked, send Abort and wait...
                    thread.Abort();
                    if (!thread.Join(250))
                    {
                        // If that hasn't worked, UdpClient.Receive() seems to be immune even to that! -- force close the socket...
                        if (udpClient != null && udpClient.Client != null) 
                        { 
                            udpClient.Client.Close();
                            if (!thread.Join(250))
                            {
                                ; // If that hasn't worked, give up!
                            }
                        }
                    }
                }
                thread = null;
            }
        }


        // UDP listening thread
        private void Listener()
        {
            while (!quitReceiver)
            {
                try
                {
                    // Receive UDP packet
                    byte[] buffer = udpClient.Receive(ref endPoint);

                    if (quitReceiver) { break; }

                    // Create OscData object
                    OscData messageOrBundle = null;
                    try
                    {
                        messageOrBundle = OscData.FromByteArray(buffer);
                    }
                    catch (Exception ex)
                    {
                        Console.Error.WriteLine("ERROR: Problem creating OscData from message (" + ex.Message + ").");
                    }

                    if (messageOrBundle != null)
                    {
                        if (messageOrBundle is OscMessage)
                        {
                            OscMessage message = (OscMessage)messageOrBundle;
                            DateTime timestamp = DateTime.UtcNow;
                            OnReceivedMessage(new OscMessageEventArgs(timestamp, message));
                        }
                        else if (messageOrBundle is OscBundle)
                        {
                            OscBundle bundle = (OscBundle)messageOrBundle;
                            DateTime timestamp = (bundle.Timestamp == OscData.TIMESTAMP_NOW) ? DateTime.UtcNow : OscData.DateTimeFromTimestamp(bundle.Timestamp);
                            OnReceivedBundle(new OscBundleEventArgs(timestamp, bundle));
                        }
                    }
                }
                catch (ThreadInterruptedException)
                {
                    Console.Error.WriteLine("WARNING: ThreadInterruptedException in Listener...");
                    if (quitReceiver) { break; }
                }
                catch (SocketException)
                {
                    Console.Error.WriteLine("WARNING: SocketException in Listener...");
                    if (quitReceiver) { break; }
                }
            }
            udpClient.Close();
        }

        // Dispose
        public void Dispose()
        {
            Stop();
        }


    }
    #endregion
}


