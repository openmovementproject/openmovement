// SLIP reading routines (and other utility functions)
// Dan Jackson, 2010-2011

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace TeddiGui
{
    public class Slip
    {

        // SLIP constants
        public const byte SLIP_END = 0xC0;                   // 192 End of packet indicator
        public const byte SLIP_ESC = 0xDB;                   // 219 Escape character, next character will be a substitution
        public const byte SLIP_ESC_END = 0xDC;               // 220 Escaped substitution for the END data byte
        public const byte SLIP_ESC_ESC = 0xDD;               // 221 Escaped substitution for the ESC data byte

        // Remove and return a SLIP (RFC 1055) encoded packet from the serial port
        public static byte[] ExtractSlipPacket(List<byte> originalBuffer)
        {
            int endIndex = originalBuffer.IndexOf(SLIP_END);
            if (endIndex < 0)
            {
                return null;
            }

            // Create a new buffer that contains the encoded packet
            byte[] buffer = new byte[endIndex];
            originalBuffer.CopyTo(0, buffer, 0, endIndex);
            originalBuffer.RemoveRange(0, endIndex + 1);

            List<byte> packet = new List<byte>();
            bool lastWasEscape = false;
            for (int i = 0; i < buffer.Length; i++)
            {
                byte c = buffer[i];
                if (c == SLIP_END) { break; }
                else if (!lastWasEscape && c == SLIP_ESC) { lastWasEscape = true; }
                else
                {
                    if (lastWasEscape)
                    {
                        // Substitute escaped char
                        if (c == SLIP_ESC_END) { c = SLIP_END; lastWasEscape = false; }
                        else if (c == SLIP_ESC_ESC) { c = SLIP_ESC; lastWasEscape = false; }
                        else if (c == SLIP_ESC)
                        {
                            Console.WriteLine("SLIP: Invalid double-ESC");
                            c = SLIP_ESC;
                            lastWasEscape = true;
                        }
                        else
                        {
                            Console.WriteLine("SLIP: Invalid code after ESC: " + c);
                            //c = c;               // Invalid escaped char!
                            lastWasEscape = false;
                        }
                    }
                    // Add character to packet
                    packet.Add(c);
                }
            }
            if (lastWasEscape)
            {
                Console.WriteLine("SLIP: Invalid packet END after ESC");
            }

            byte[] packetArray = packet.ToArray();
            return packetArray;
        }


        // Return a SLIP (RFC 1055) encoded packet
        public static byte[] CreateSlipPacket(byte[] data)
        {
            List<byte> buffer = new List<byte>();
            buffer.Add(SLIP_END);
            for (int i = 0; i < data.Length; i++)
            {
                byte c = data[i];
                if (c == SLIP_END) { buffer.Add(SLIP_ESC); buffer.Add(SLIP_ESC_END); }
                else if (c == SLIP_ESC) { buffer.Add(SLIP_ESC); buffer.Add(SLIP_ESC_ESC); }
                else { buffer.Add(c); }
            }
            buffer.Add(SLIP_END);
            return buffer.ToArray();
        }



        // Remove and return a CR/LF-delimited packet from a byte list
        public static byte[] ExtractLinePacket(List<byte> originalBuffer)
        {
            const byte CR = 0x0D;                   // 13 Carriage return
            const byte LF = 0x0A;                   // 10 Line feed

            int crIndex = originalBuffer.IndexOf(CR);
            int lfIndex = originalBuffer.IndexOf(LF);

            if (lfIndex == 0) { originalBuffer.RemoveRange(0, 1); crIndex = originalBuffer.IndexOf(CR); lfIndex = originalBuffer.IndexOf(LF); }    // Fix for LF arriving after CR

            int endIndex = -1;
            if (crIndex < 0 && lfIndex < 0) { return null; }
            else if (crIndex < 0) { endIndex = lfIndex; }
            else if (lfIndex < 0) { endIndex = crIndex; }
            else { endIndex = Math.Min(crIndex, lfIndex); }

            int consumeIndex = (crIndex != -1 && lfIndex != -1 && lfIndex == crIndex + 1) ? endIndex + 1 : endIndex;

            // Create a new buffer that contains the encoded packet
            byte[] buffer = new byte[endIndex];
            originalBuffer.CopyTo(0, buffer, 0, endIndex);
            originalBuffer.RemoveRange(0, consumeIndex + 1);
            return buffer;
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
}
