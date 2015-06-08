using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace WaxLib
{
    public interface PacketConverter<T>
    {
        T FromBinary(byte[] buffer, DateTime timestamp);
        byte[] ToBinary(T value);

        T FromStomp(string value);
        string ToStomp(T value);

        T FromOscBundle(OscBundle oscBundle, string topic);
        OscBundle ToOscBundle(T value, string topic);

        string ToLog(T value);

        string GetTopicPostfix(T value);

        string GetDefaultTopic(bool receiving);
    }


}
