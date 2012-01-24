using System;

namespace OmApiNet
{
    public sealed class OmException : Exception
    {
        public OmException(int code, string message) : base(OmApi.OmErrorString(code) + " - " + message) { }
        public OmException(int code, string message, Exception inner) : base(OmApi.OmErrorString(code) + " - " + message, inner) { }
    }
}

