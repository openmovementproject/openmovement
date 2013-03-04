using System;
using System.Collections.Generic;
using System.Linq;
using System.Media;
using System.Text;

namespace TeddiGui
{
    public class MotionTracker
    {
        public int Id { get; protected set; }
        public string Task { get; protected set; }

        uint count = 0;
        float average = 512.0f;
        bool signal = false;

        public MotionTracker(int id, string task)
        {
            Id = id;
            Task = task;
        }

        public void NewPacket(Packet packet)
        {
            foreach (ushort v in packet.Pir)
            {
                const float weight = 0.99f;
                const float latch = 50.0f;

                average = weight * average + (1.0f - weight) * v;
                if (count == 0) { average = v; }
                count++;

                float diff = Math.Abs(average - v);
                bool newSignal = (diff > latch);

//Console.WriteLine("MOTION: #" + Id + " = " + v + " - " + average + " = " + diff + " (" + latch + ") == " + newSignal + "");

                if (newSignal && !signal && count > 48)
                {
                    count = 1;  // reset
                    try
                    {
                        if (Task.ToLower().EndsWith(".wav"))
                        {
                            SoundPlayer simpleSound = new SoundPlayer(Task);
                            simpleSound.Play();
                        }
                        else
                        {
                            Console.Error.WriteLine("WARNING: Unhandled task: " + Task);
                        }
                    }
                    catch (Exception e)
                    {
                        Console.Error.WriteLine("ERROR: Problem running task: " + e.Message);
                    }
                }
                signal = newSignal;
            }
        }

    }
}
