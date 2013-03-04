using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Drawing.Drawing2D;

namespace TeddiGui
{
    public partial class GraphVisualizer : IVisualizer
    {
        public GraphVisualizer(Manager manager) : base(manager)
        {
            InitializeComponent();
        }

        // draws to the HistoryPanel
        public static void DrawGraph(Manager manager, int num, Graphics g, Rectangle rect, Device device)
        {
            lock (manager)
            {

                List<PointF> accelXPoints = new List<PointF>();
                List<PointF> accelYPoints = new List<PointF>();
                List<PointF> accelZPoints = new List<PointF>();
                List<PointF> accelHPoints = new List<PointF>();
                List<PointF> accelVPoints = new List<PointF>();

                Pen penX = new Pen(Color.FromArgb(0x80, Color.Pink), 4.0f); Pen penX2 = new Pen(Color.FromArgb(0x80, penX.Color), 1.0f); penX2.DashStyle = DashStyle.Dash;
                Pen penY = new Pen(Color.FromArgb(0x80, Color.LightGreen), 4.0f); Pen penY2 = new Pen(Color.FromArgb(0x80, penY.Color), 1.0f); penY2.DashStyle = DashStyle.Dash;
                Pen penZ = new Pen(Color.FromArgb(0x80, Color.LightBlue), 4.0f); Pen penZ2 = new Pen(Color.FromArgb(0x80, penZ.Color), 1.0f); penZ2.DashStyle = DashStyle.Dash;
                Pen penH = new Pen(Color.FromArgb(0x80, Color.LightSlateGray), 4.0f); Pen penH2 = new Pen(Color.FromArgb(0x80, penZ.Color), 1.0f); penZ2.DashStyle = DashStyle.Dash;
                Pen penV = null, penV2 = null;
                //penV = new Pen(Color.FromArgb(0x80, Color.Gray), 2.0f); penV2 = new Pen(Color.FromArgb(0x80, penV.Color), 1.0f); penV2.DashStyle = DashStyle.Dash;

                penX.LineJoin = LineJoin.Round; penX2.LineJoin = LineJoin.Round;
                penY.LineJoin = LineJoin.Round; penY2.LineJoin = LineJoin.Round;
                penZ.LineJoin = LineJoin.Round; penZ2.LineJoin = LineJoin.Round;
                penH.LineJoin = LineJoin.Round; penH2.LineJoin = LineJoin.Round;
                if (penV != null) { penV.LineJoin = LineJoin.Round; }
                if (penV2 != null) { penV2.LineJoin = LineJoin.Round; }

                Pen axis = new Pen(Color.FromArgb(0x80, Color.White), 1.0f);
                Pen axis2 = new Pen(Color.FromArgb(0x80, axis.Color), 1.0f); axis2.DashStyle = DashStyle.Custom; axis2.DashPattern = new float[] { 8.0f, 4.0f };

                long latestTime = long.MinValue, earliestTime = long.MaxValue; // latestTime - (WAX.MAX_HISTORY * (1000 / 10));
                int min = -1, max = 1;
                float minV = 0.0f, maxV = 0.00001f;
                penV = new Pen(Color.FromArgb(0x80, Color.Orange), 4.0f); penV2 = new Pen(Color.FromArgb(0x80, penV.Color), 1.0f); penV2.DashStyle = DashStyle.Dash;
                minV = float.MaxValue; maxV = float.MinValue; min = int.MaxValue; max = int.MinValue;

                foreach (Sample sample in device.Samples)
                {
                    if ((long)sample.Index < earliestTime) { earliestTime = (long)sample.Index; }
                    if ((long)sample.Index > latestTime) { latestTime = (long)sample.Index; }
                    if (sample.X < min) { min = sample.X; }
                    if (sample.X > max) { max = sample.X; }
                    if (sample.Y < min) { min = sample.Y; }
                    if (sample.Y > max) { max = sample.Y; }
                    if (sample.Z < min) { min = sample.Z; }
                    if (sample.Z > max) { max = sample.Z; }
                    if (sample.H < min) { min = sample.H; }
                    if (sample.H > max) { max = sample.H; }
                    if (sample.V < minV) { minV = sample.V; }
                    if (sample.V > maxV) { maxV = sample.V; }
                }

                //if (max > -min) { min = -max; }
                //if (min < -max) { max = -min; }

                { min = -512; max = 512; }  // Fixed scale for XYZ, auto V
                //            else if (scaleMode == 2) { minV = min = (int)Math.Min(min, minV); maxV = max = (int)Math.Max(max, maxV); }      // Same scale for XYZW (non-accel data)

                // Fix time
                //earliestTime = latestTime - (WAX.MAX_HISTORY * (1000 / 10));
                earliestTime = latestTime - 500;

                //g.TranslateTransform(Translation.X, Translation.Y);
                g.SmoothingMode = SmoothingMode.HighQuality;
                //g.FillRectangle(((num & 1) == 0) ? SystemBrushes.ControlLight : SystemBrushes.Control, rect);
                g.FillRectangle(((num & 1) == 0) ? Brushes.Black : new SolidBrush(Color.FromArgb(33, 33, 33)), rect);

                /*
                if (scaleMode == 1)
                {
                    float y;
                    int range = max - min;
                    if (range == 0) { range = 1; }

                    // 0g
                    y = rect.Top + rect.Height - ((0 - min) * rect.Height / range);
                    g.DrawLine(axis, (float)rect.Left, y, (float)rect.Right, y);

                    if (scaleMode != 2)
                    {
                        // -1g
                        y = rect.Top + rect.Height - ((-256 - min) * rect.Height / range);
                        g.DrawLine(axis2, (float)rect.Left, y, (float)rect.Right, y);

                        // +1g
                        y = rect.Top + rect.Height - ((256 - min) * rect.Height / range);
                        g.DrawLine(axis2, (float)rect.Left, y, (float)rect.Right, y);
                    }
                }
                */

                Brush fontBrush = Brushes.Black;
                Font font = DefaultFont;
                fontBrush = Brushes.White;
                font = new Font("Courier New", 14.0f);  // FontFamily.GenericMonospace, 

                if (latestTime > earliestTime)
                {
                    ulong lastTime = 0;
                    foreach (Sample sample in device.Samples)
                    {
                        if (!sample.valid) { continue; }
                        long missing = (long)(sample.Index - lastTime);
                        lastTime = sample.Index;

                        float x = rect.Left + ((long)sample.Index - earliestTime) * (float)rect.Width / (latestTime - earliestTime);

                        // Cope with discontinuity
                        float lxx = 0, lx = 0, ly = 0, lz = 0, lh = 0, lv = 0;
                        if (missing > 1 && accelXPoints.Count >= 2)
                        {
                            if (accelXPoints.Count >= 2) { g.DrawLines(penX, accelXPoints.ToArray()); }
                            if (accelYPoints.Count >= 2) { g.DrawLines(penY, accelYPoints.ToArray()); }
                            if (accelZPoints.Count >= 2) { g.DrawLines(penZ, accelZPoints.ToArray()); }
                            if (accelHPoints.Count >= 2) { g.DrawLines(penH, accelHPoints.ToArray()); }
                            if (penV != null && accelVPoints.Count >= 2) { g.DrawLines(penV, accelVPoints.ToArray()); }
                            lxx = accelXPoints[accelXPoints.Count - 1].X;
                            lx = accelXPoints[accelXPoints.Count - 1].Y;
                            ly = accelYPoints[accelYPoints.Count - 1].Y;
                            lz = accelZPoints[accelZPoints.Count - 1].Y;
                            lh = accelHPoints[accelHPoints.Count - 1].Y;
                            lv = accelVPoints[accelVPoints.Count - 1].Y;
                            accelXPoints.Clear();
                            accelYPoints.Clear();
                            accelZPoints.Clear();
                            accelHPoints.Clear();
                            accelVPoints.Clear();
                        }

                        if ((max - min) == 0) { max++; }
                        if ((maxV - minV) == 0) { maxV++; }

                        float xy = rect.Top + rect.Height - ((sample.X - min) * rect.Height / (max - min));
                        float yy = rect.Top + rect.Height - ((sample.Y - min) * rect.Height / (max - min));
                        float zy = rect.Top + rect.Height - ((sample.Z - min) * rect.Height / (max - min));
                        float hy = rect.Top + rect.Height - ((sample.H - min) * rect.Height / (max - min));
                        float vy = rect.Top + rect.Height - ((sample.V - minV) * rect.Height / (maxV - minV));

                        if (xy < rect.Top) { xy = rect.Top; } if (xy > rect.Top + rect.Height - 1) { xy = rect.Top + rect.Height - 1; }
                        if (yy < rect.Top) { yy = rect.Top; } if (yy > rect.Top + rect.Height - 1) { yy = rect.Top + rect.Height - 1; }
                        if (zy < rect.Top) { zy = rect.Top; } if (zy > rect.Top + rect.Height - 1) { zy = rect.Top + rect.Height - 1; }
                        if (hy < rect.Top) { hy = rect.Top; } if (hy > rect.Top + rect.Height - 1) { hy = rect.Top + rect.Height - 1; }
                        if (vy < rect.Top) { vy = rect.Top; } if (vy > rect.Top + rect.Height - 1) { vy = rect.Top + rect.Height - 1; }

                        // Draw dashed continuity lines
                        if (missing > 1)
                        {
                            if (lxx != 0 || lx != 0) { g.DrawLine(penX2, lxx, lx, x, xy); }
                            if (lxx != 0 || ly != 0) { g.DrawLine(penY2, lxx, ly, x, yy); }
                            if (lxx != 0 || lz != 0) { g.DrawLine(penZ2, lxx, lz, x, zy); }
                            if (lxx != 0 || lh != 0) { g.DrawLine(penH2, lxx, lh, x, hy); }
                            if (lxx != 0 || lv != 0) { if (penV2 != null) { g.DrawLine(penV2, lxx, lv, x, vy); } }
                        }

                        accelXPoints.Add(new PointF(x, xy));
                        accelYPoints.Add(new PointF(x, yy));
                        accelZPoints.Add(new PointF(x, zy));
                        accelHPoints.Add(new PointF(x, hy));
                        accelVPoints.Add(new PointF(x, vy));
                    }

                    if (accelXPoints.Count >= 2) { g.DrawLines(penX, accelXPoints.ToArray()); }
                    if (accelYPoints.Count >= 2) { g.DrawLines(penY, accelYPoints.ToArray()); }
                    if (accelZPoints.Count >= 2) { g.DrawLines(penZ, accelZPoints.ToArray()); }
                    if (accelHPoints.Count >= 2) { g.DrawLines(penH, accelHPoints.ToArray()); }
                    if (accelVPoints.Count >= 2 && penV != null) { g.DrawLines(penV, accelVPoints.ToArray()); }
                }

                int deviceId = device.Id;
                string label = deviceId.ToString();
                SizeF labelSize = g.MeasureString(label, font);
                g.DrawString(label, font, fontBrush, (float)rect.Width - labelSize.Width - 5, (float)rect.Y + 5);
                g.DrawString(label, font, fontBrush, (float)rect.X + 5, (float)rect.Y + 5);

                if (device.Samples.Count > 0)
                {
                    Sample lastSample = device.Samples[device.Samples.Count - 1];
                    string t = "pir=" + lastSample.X.ToString() + ", temp=" + lastSample.Temperature().ToString("F2") + ", light=" + lastSample.Z.ToString() + ", hum=" + lastSample.Humidity().ToString("F2") + ", sound=" + lastSample.V.ToString() + ", batt=" + lastSample.BattVolts() + " (" + lastSample.BattPercent() + "%)";
                    g.DrawString(t, font, fontBrush, (float)rect.X + 50, (float)rect.Y + 5 + 0 * font.Height);
                    /*
                    g.DrawString("motion = " + lastSample.X.ToString(), font, fontBrush, (float)rect.X + 5, (float)rect.Y + 5 + 1 * font.Height);
                    g.DrawString("temp   = " + lastSample.Y.ToString(), font, fontBrush, (float)rect.X + 5, (float)rect.Y + 5 + 2 * font.Height);
                    g.DrawString("light  = " + lastSample.Z.ToString(), font, fontBrush, (float)rect.X + 5, (float)rect.Y + 5 + 3 * font.Height);
                    g.DrawString("sound  = " + lastSample.V.ToString(), font, fontBrush, (float)rect.X + 5, (float)rect.Y + 5 + 4 * font.Height);
                    */
                }
            }
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            if (manager != null)
            {
                int numPanels;
                Device[] devices;

                // Ensure we can access the collection without interruption
                lock (manager)
                {
                    devices = manager.Devices.Values.ToArray();
                    numPanels = devices.Length;

                    // Sort devices
                    int[] sortOrder = new int[devices.Length];
                    for (int i = 0; i < devices.Length; i++)
                    {
                        sortOrder[i] = devices[i].Id;
                    }
                    Array.Sort(sortOrder, devices);
                }

                if (numPanels < 2) { numPanels = 2; }

                Graphics g = e.Graphics;
                int panelNum = 0;
                foreach (Device device in devices)
                {
                    int x1 = 0;
                    int x2 = this.Width - 1;
                    int y1 = this.Height * panelNum / numPanels;
                    int y2 = this.Height * (panelNum + 1) / numPanels - 1;
                    DrawGraph(manager, panelNum, g, new Rectangle(x1, y1, x2 - x1, y2 - y1), device);
                    panelNum++;
                }

            }

        }

    }
}
