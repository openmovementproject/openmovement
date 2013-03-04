using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace TeddiGui
{
    public partial class MapVisualizer : IVisualizer
    {
        string title = "";
        string imageFile = "";

        private const float MAX_CONNECTION_AGE = 130.0f;

        IDictionary<int, Router> routers = new Dictionary<int, Router>();
        IDictionary<int, Sensor> sensors = new Dictionary<int, Sensor>();
        List<Node> nodes = new List<Node>();
        IDictionary<int, List<Connection>> connections = new Dictionary<int, List<Connection>>();

        // 2.000 White, 1.750 Magenta, 1.500 Red, 1.250 Yellow, 1.000 Green, 0.750 Cyan, 0.500 Blue, 0.250 DarkGray, 0.000 Black
        public static readonly Color[] RAINBOW_GRADIENT = new Color[] { Color.DarkSlateGray, Color.DarkGray, Color.Blue, Color.Cyan, Color.Green, Color.Yellow, Color.Red, Color.Magenta, Color.White };
        public static readonly Color[] GRAY_GRADIENT = new Color[] { Color.Black, Color.White };

        public static Color[] temperatureGradient = ColorGradient(RAINBOW_GRADIENT, 256);
        public static Color[] lightGradient = ColorGradient(GRAY_GRADIENT, 256);

        protected static Color[] ColorGradient(Color[] colorStops, int levels, int a = 255)
        {
            Color[] colors = new Color[levels];
            for (int p = 0; p < colors.Length; p++)
            {
                float prop = (float)p / (colors.Length - 1);
                int i0 = p * (colorStops.Length - 1) / colors.Length;
                int i1 = i0 + 1;
                Color c0 = colorStops[i0];
                Color c1 = colorStops[i1];
                float mix = (p - (i0 * (float)(colors.Length - 1) / (colorStops.Length - 1))) / ((float)(colors.Length - 1) / (colorStops.Length - 1));
                int r = (int)((1.0f - mix) * c0.R + mix * c1.R);
                int g = (int)((1.0f - mix) * c0.G + mix * c1.G);
                int b = (int)((1.0f - mix) * c0.B + mix * c1.B);
                colors[p] = Color.FromArgb(a, (int)r, (int)g, (int)b);
            }
            return colors;
        }


        public MapVisualizer(Manager manager) : base(manager)
        {
            InitializeComponent();
            LoadFromFile(null);
            this.MouseMove += MapVisualizer_MouseMove;
            this.MouseLeave += MapVisualizer_MouseLeave;
        }

        private Node selectedNode = null;
        public Node Selected { get { return selectedNode; } protected set { selectedNode = value; Invalidate(); } }

        void MapVisualizer_MouseLeave(object sender, EventArgs e)
        {
            Selected = null;
        }

        void MapVisualizer_MouseMove(object sender, MouseEventArgs e)
        {
            if (this.Capture)
            {
                if (selectedNode != null)
                {
                    selectedNode.X = e.Location.X + mouseDownRelative.X;
                    selectedNode.Y = e.Location.Y + mouseDownRelative.Y;
                    Invalidate();
                }
            }
            else
            {
                Node selectedNode = null;
                float closest = float.MaxValue;
                foreach (Node node in nodes)
                {
                    int nx = node.X, ny = node.Y;
                    float dist = (float)Math.Sqrt((e.X - nx) * (e.X - nx) + (e.Y - ny) * (e.Y - ny));
                    if (dist < closest)
                    {
                        closest = dist;
                        selectedNode = node;
                    }
                }
                if (closest > 50) { selectedNode = null; }
                Selected = selectedNode;
            }
        }

        string filename;

        public static string RelativePath(string fromPath, string toPath)
        {
            return Uri.UnescapeDataString((new Uri(fromPath)).MakeRelativeUri(new Uri(toPath)).ToString());
        }


        public bool SaveToFile(string filename = null)
        {
            if (filename == null) { filename = this.filename; }

            StringBuilder data = new StringBuilder();

            data.AppendLine("; This file is machine written");
            data.AppendLine("");
            data.AppendLine("Title," + title);
            data.AppendLine("");
//data.AppendLine("Image," + RelativePath(filename, imageFile));
            data.AppendLine("Image," + imageFile);
            data.AppendLine("");
            foreach (Router router in routers.Values)
            {
                data.AppendLine("Router," + router.Id + "," + router.X + "," + router.Y + "," + router.A + "," + router.Label + "");
            }
            data.AppendLine("");
            int[] keys = connections.Keys.ToArray();
            Array.Sort(keys);
            foreach (int a in keys)
            {
                List<Connection> connectionList = connections[a];
                foreach (Connection connection in connectionList)
                {
                    data.AppendLine("Connection," + connection.A + "," + connection.B + "," + (connection.Parent ? 1 : 0) + "");
                }
            }
            data.AppendLine("");
            foreach (Sensor sensor in sensors.Values)
            {
                data.AppendLine("Sensor," + sensor.Id + "," + sensor.X + "," + sensor.Y + "," + sensor.A + "," + sensor.Label + "");
            }
            data.AppendLine("");

            try
            {
                System.IO.File.WriteAllText(filename, data.ToString());
            }
            catch (Exception e)
            {
                Console.Error.WriteLine("EXCEPTION: Problem saving file '" + filename + "' - " + e.Message + "");
            }

            return true;
        }

        public bool LoadFromFile(string filename)
        {
            string data = "";

            this.filename = filename;

            if (filename == null || filename.Length == 0)
            {
                Console.WriteLine("NOTE: Clearing map.");
            }
            else if (!System.IO.File.Exists(filename))
            {
                Console.Error.WriteLine("ERROR: Data file does not exist '" + filename + "'.");
            }
            else
            {
                try
                {
                    data = System.IO.File.ReadAllText(filename);
                }
                catch (Exception e)
                {
                    Console.Error.WriteLine("EXCEPTION: Problem loading file '" + filename + "' - " + e.Message + "");
                }
            }

            string[] lines = data.Split(new char[] { '\r', '\n' }, StringSplitOptions.RemoveEmptyEntries);
            routers.Clear();
            sensors.Clear();
            connections.Clear();
            imageFile = "";
            title = "";
            string sectionName = "";
            foreach (string line in lines)
            {
                string text = line.Trim();
                if (text.Length == 0 || text[0] == '#' || text[0] == ';') { continue; }
                if (text.StartsWith("["))
                {
                    text = text.Substring(1).Trim();
                    if (text.EndsWith("]")) { text = text.Substring(0, text.Length - 1).Trim(); }
                    sectionName = text;
                }
                else
                {
                    string[] tokens = text.Split(new char[] { '=', ',' });
                    if (tokens[0].Equals("Router", StringComparison.InvariantCultureIgnoreCase) && tokens.Length > 3)
                    {
                        int id = -1, x = -1, y = -1, a = -1;
                        string label = "";
                        int.TryParse(tokens[1], out id);
                        int.TryParse(tokens[2], out x);
                        int.TryParse(tokens[3], out y);
                        if (tokens.Length > 4) { int.TryParse(tokens[4], out a); }
                        if (tokens.Length > 5) { label = tokens[5]; }
                        routers.Add(id, new Router(id, x, y, a, label));
                    }
                    else if (tokens[0].Equals("Sensor", StringComparison.InvariantCultureIgnoreCase) && tokens.Length > 3)
                    {
                        int id = -1, x = -1, y = -1, a = -1;
                        string label = "";
                        int.TryParse(tokens[1], out id);
                        int.TryParse(tokens[2], out x);
                        int.TryParse(tokens[3], out y);
                        if (tokens.Length > 4) { int.TryParse(tokens[4], out a); }
                        if (tokens.Length > 5) { label = tokens[5]; }
                        sensors.Add(id, new Sensor(id, x, y, a, label));
                    }
                    else if (tokens[0].Equals("Connection", StringComparison.InvariantCultureIgnoreCase) && tokens.Length > 2)
                    {
                        int a = -1, b = -1;
                        int type = 1;     // Old format only had parent connections
                        int.TryParse(tokens[1], out a);
                        int.TryParse(tokens[2], out b);
                        if (tokens.Length > 3) { int.TryParse(tokens[3], out type); }
                        if (!connections.ContainsKey(a)) { connections.Add(a, new List<Connection>()); }
                        connections[a].Add(new Connection(a, b, (type == 1)));
                    }
                    else if (tokens[0].Equals("Image", StringComparison.InvariantCultureIgnoreCase) && tokens.Length > 1)
                    {
                        imageFile = tokens[1];
                    }
                    else if (tokens[0].Equals("Title", StringComparison.InvariantCultureIgnoreCase) && tokens.Length > 1)
                    {
                        title = tokens[1];
                    }
                    else if (tokens[0].Trim().Length == 0)
                    { ; }
                    else
                    {
                        Console.WriteLine("ERROR: Ignoring unknown data line type: " + line);
                    }
                }
            }
            foreach (Sensor sensor in sensors.Values) { nodes.Add(sensor); }
            foreach (Router router in routers.Values) { nodes.Add(router); }

            this.BackgroundImage = null;
            if (imageFile != null && imageFile.Length > 0)
            {
                string f = imageFile;

                if (!System.IO.File.Exists(f))
                {
                    f = System.IO.Path.Combine(System.IO.Path.GetDirectoryName(filename), f);
                }

                if (System.IO.File.Exists(f))
                {
                    this.BackgroundImage = Bitmap.FromFile(f);
                }
            }

            return true;
        }

        protected void RenewConnection(ushort thisId, ushort otherId, bool parent, float age)
        {
            if (thisId == 0xffff || otherId == 0xffff) { return; }  // Ignore unknown source/dest

            List<Connection> connectionList;
            if (connections.ContainsKey(thisId))
            {
                connectionList = connections[thisId];
            }
            else
            {
                connectionList = new List<Connection>();
                connections.Add(thisId, connectionList);
                Console.WriteLine("CONNECTION: (New source " + thisId + ")");
            }

            Connection foundConnection = null;
            foreach (Connection connection in connectionList)
            {
                if (connection.B == otherId && connection.Parent == parent)
                {
                    foundConnection = connection;
                    break;
                }
            }

            if (foundConnection == null)
            {
                if (age < MAX_CONNECTION_AGE)
                {
                    foundConnection = new Connection(thisId, otherId, parent);
                    connectionList.Add(foundConnection);
                    Console.WriteLine("CONNECTION: New from " + thisId + " -> " + otherId + " (" + parent + ") (t=" + foundConnection.DataAge + ")");
                }
            }

            if (foundConnection != null)
            {
                foundConnection.DataAge = age;
//Console.WriteLine("CONNECTION: " + thisId + " -> " + otherId + " (" + parent + ")   @" + foundConnection.DataAge + " (" + age + ")");
            }

        }

        protected void UpdateModel()
        {
            if (manager != null)
            {
                // Ensure we can access the collection without interruption
                lock (manager)
                {
                    // Build a map of router short addresses
                    IDictionary<ushort, ushort> shortAddressMap = new Dictionary<ushort, ushort>();
                    foreach (Device device in manager.Devices.Values)
                    {
                        if (device.ShortAddress != 0xffff)
                        {
                            ushort altId = (ushort)Device.RouterAltId(device.ShortAddress);
                            ushort id = (ushort)device.Id;
                            if (!shortAddressMap.ContainsKey(altId))
                            {
                                shortAddressMap.Add(altId, id);
                            }
                        }
                    }

                    // Update sensors
                    foreach (Sensor sensor in sensors.Values)
                    {
                        if (manager.Devices.ContainsKey(sensor.Id))
                        {
                            Device device = manager.Devices[sensor.Id];
                            sensor.Age = device.DataAge;
                            sensor.ParentAddress = device.ParentAddress;
                            RenewConnection((ushort)sensor.Id, sensor.ParentAddress, true, sensor.Age);
                        }
                    }

                    // Update routers
                    foreach (Router router in routers.Values)
                    {
                        if (manager.Devices.ContainsKey(router.Id))
                        {
                            Device device = manager.Devices[router.Id];
                            router.Age = device.DataAge;
                            router.ParentAddress = device.ParentAddress;

                            // Map short addresses to actual addresses
                            List<ushort> neighbourList = new List<ushort>();
                            foreach (ushort neighbourShortAddress in device.NeighbourShortAddresses)
                            {
                                if (shortAddressMap.ContainsKey(neighbourShortAddress))
                                {
                                    neighbourList.Add(shortAddressMap[neighbourShortAddress]);
//Console.WriteLine("NOTE: Alt. address for neighbour of device " + device.Id + ": #" + neighbourShortAddress + "");
                                }
                                else
                                {
//                                    Console.WriteLine("NOTE: Unknown alt. address for neighbour of device " + device.Id + ": #" + neighbourShortAddress + "");
                                }
                            }
                            ushort[] neighbours = neighbourList.ToArray();
                            router.Neighbours = neighbours;

                            // Update connections
                            RenewConnection((ushort)router.Id, router.ParentAddress, true, router.Age);
                            foreach (ushort neighbour in neighbours)
                            {
                                RenewConnection((ushort)router.Id, neighbour, false, router.Age);
                            }

                        }
                    }

                    // Remove dead connections
                    foreach (KeyValuePair<int, List<Connection>> pair in connections)
                    {
                        List<Connection> toDelete = new List<Connection>();
                        foreach (Connection connection in pair.Value)
                        {
                            if (connection.DataAge > MAX_CONNECTION_AGE)
                            {
                                toDelete.Add(connection);
                            }
                        }
                        foreach (Connection connection in toDelete)
                        {
                            Console.WriteLine("CONNECTION: Removed " + connection.A + " -> " + connection.B + " (" + connection.Parent + ") (t=" + connection.DataAge + ").");
                            pair.Value.Remove(connection);
                        }
                    }

                }
            }
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            UpdateModel();

            if (manager != null)
            {
                // Ensure we can access the collection without interruption
                lock (manager)
                {
                    Graphics g = e.Graphics;

                    foreach (KeyValuePair<int, List<Connection>> pair in connections)
                    {
                        int idA = pair.Key;
                        Node nodeA = null;
                        if (routers.ContainsKey(idA)) { nodeA = routers[idA]; }
                        if (sensors.ContainsKey(idA)) { nodeA = sensors[idA]; }
                        if (nodeA != null)
                        {
                            foreach (Connection connection in pair.Value)
                            {
                                int idB = connection.B;
                                Node nodeB = null;
                                if (routers.ContainsKey(idB)) { nodeB = routers[idB]; }
                                if (sensors.ContainsKey(idB)) { nodeB = sensors[idB]; }
                                if (nodeB != null)
                                {
                                    const float range = 60.0f;
                                    float age = connection.DataAge;
                                    if (age > range) { age = range; }
                                    if (age < 0.0f) { age = range; }

                                    float alpha = 1.0f - (age / range);
                                    alpha = 0.3f + (0.7f * alpha);

                                    Pen pen;
                                    if (connection.Parent)
                                    {
                                        pen = new Pen(Color.FromArgb((byte)(alpha * 255), Color.LightBlue));
                                        pen.Width = 2.0f;
                                        pen.DashStyle = System.Drawing.Drawing2D.DashStyle.Dot;
                                    }
                                    else
                                    {
                                        pen = new Pen(Color.FromArgb((byte)(alpha * 255), Color.LightGray));
                                        pen.Width = 1.0f;
                                        pen.DashStyle = System.Drawing.Drawing2D.DashStyle.Custom; 
                                        pen.DashPattern = new float[] { 0.5f, 8.0f };
                                    }

                                    g.DrawLine(pen, nodeA.X, nodeA.Y, nodeB.X, nodeB.Y);
                                }
                            }
                        }
                    }

                    foreach (Router router in routers.Values)
                    {
                        router.Draw(g, (router == Selected));
                    }

                    foreach (Sensor sensor in sensors.Values)
                    {
                        Device device = null;

                        if (manager.Devices.ContainsKey(sensor.Id))
                        {
                            device = manager.Devices[sensor.Id];
                        }

                        sensor.Draw(g, device, (sensor == Selected));
                    }

                    Node selectedNode = Selected;
                    if (selectedNode != null)
                    {
                        if (selectedNode is Sensor && manager.Devices.ContainsKey(selectedNode.Id))
                        {
                            GraphVisualizer.DrawGraph(manager, 0, g, new Rectangle(0, this.Height - 100, this.Width, 100), manager.Devices[selectedNode.Id]);
                        }

                        Brush fontBrush = Brushes.White;
                        Font font = new Font("Courier New", 10.0f);  // FontFamily.GenericMonospace, 
                        string label = "" + selectedNode.Id + " - " + selectedNode.Label;
                        SizeF labelSize = g.MeasureString(label, font);
                        g.DrawString(label, font, fontBrush, this.Width / 2 - labelSize.Width / 2, this.Height - font.Height);

                    }

                }
            }

        }

        Point mouseDownRelative = Point.Empty;

        private void MapVisualizer_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == System.Windows.Forms.MouseButtons.Left)
            {
                mouseDownRelative = e.Location;
                if (selectedNode != null)
                {
                    mouseDownRelative.X = selectedNode.X - mouseDownRelative.X;
                    mouseDownRelative.Y = selectedNode.Y - mouseDownRelative.Y;
                    this.Capture = true;
                }
            }
        }

        private void MapVisualizer_MouseUp(object sender, MouseEventArgs e)
        {
            if (this.Capture)
            {
                this.Capture = false;
            }
        }

        private void MapVisualizer_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (selectedNode != null)
            {
                int a = 15 * (int)((selectedNode.A + 7.5) / 15);
                if (a >= 360) { a -= 360; }
                if (a < 0) { a += 360; }
                if (e.KeyChar == '=' || e.KeyChar == '+') { selectedNode.A = a + 15; }
                if (e.KeyChar == '-') { selectedNode.A = a - 15; }
                Invalidate();
            }
        }



    }


    public abstract class Node
    {
        public int Id { get; protected set; }
        public int X { get; set; }
        public int Y { get; set; }
        public int A { get; set; }
        public string Label { get; protected set; }
        public float Age { get; set; }
        public ushort ParentAddress { get; set; }

        public Node(int id, int x, int y, int a, string label)
        {
            Id = id;
            X = x;
            Y = y;
            A = a;
            Label = label;
            Age = -1.0f;
            ParentAddress = 0xffff;
        }
    }

    public class Sensor : Node
    {
        public Sensor(int id, int x, int y, int a, string label)
            : base(id, x, y, a, label)
        {
            ;
        }

        public void Draw(Graphics g, Device device, bool selected)
        {
            float radius = 5.0f;
            Brush circleBrush = new SolidBrush(Color.FromArgb(128, Color.Red));
            Pen circlePen = new Pen(Color.FromArgb(192, Color.White));

            g.FillEllipse(circleBrush, X - radius / 2, Y - radius / 2, radius, radius);
            g.DrawEllipse(circlePen, X - radius / 2, Y - radius / 2, radius, radius);

            float radius2 = selected ? 50.0f : 35.0f;
            Brush wedgeBrush = new SolidBrush(Color.FromArgb(16, Color.White));
            Pen wedgePen = new Pen(Color.FromArgb(192, Color.White));
            float x1 = X + radius2 * (float)Math.Sin((A - 45) / 180.0f * (float)Math.PI);
            float y1 = Y + radius2 * (float)-Math.Cos((A - 45) / 180.0f * (float)Math.PI);
            float x2 = X + radius2 * (float)Math.Sin((A + 45) / 180.0f * (float)Math.PI);
            float y2 = Y + radius2 * (float)-Math.Cos((A + 45) / 180.0f * (float)Math.PI);
            float xc = X + radius2 * (float)Math.Sin(A / 180.0f * (float)Math.PI);
            float yc = Y + radius2 * (float)-Math.Cos(A / 180.0f * (float)Math.PI);

            g.FillPolygon(wedgeBrush, new PointF[] { new PointF(X, Y), new PointF(x1, y1), new PointF(x2, y2) });
            g.DrawLine(wedgePen, X, Y, x1, y1);
            g.DrawLine(wedgePen, X, Y, x2, y2);

            Brush fontBrush = Brushes.White;
            if (device == null) { fontBrush = Brushes.LightGray; }
            Font font = new Font("Courier New", 10.0f);  // FontFamily.GenericMonospace, 

            string label = Label;
            if (label == null || label.Length == 0) { label = "S" + Id + ""; }
            SizeF labelSize = g.MeasureString(label, font);

            if (A >= 2.5 * 45 && A < 5.5 * 45)
            {
                // Above
                g.DrawString(label, font, fontBrush, X - labelSize.Width / 2, Y - radius - font.Height);
            }
            else if ((A >= 0 && A < 1.5 * 45) || A >= 6.5 * 45)
            {
                // Below
                g.DrawString(label, font, fontBrush, X - labelSize.Width / 2, Y + radius);
            }
            else if (A >= 0.5 * 45 && A < 3 * 45)
            {
                // Left
                g.DrawString(label, font, fontBrush, X - labelSize.Width - radius, Y - font.Height / 2);
            }
            else if (A >= 4.5 * 45 && A <= 7.5 * 45)
            {
                // Right
                g.DrawString(label, font, fontBrush, X + radius, Y - font.Height / 2);
            }
            else
            {
                // Below
                g.DrawString(label, font, fontBrush, X - labelSize.Width / 2, Y + radius);
            }

            DrawBlob(g, xc, yc, device, selected);
        }

        void DrawBlob(Graphics g, float xc, float yc, Device device, bool selected)
        {
            if (device == null) { return; }

            int numPoints = 4 * (6 * 10);

            // Draw outer region
            {
                Color color = Color.White;
                if (device.Samples.Count > 0 && device.Samples[device.Samples.Count - 1].valid)
                {
                    float temp = device.Samples[device.Samples.Count - 1].Temperature();
                    int gi = (int)(7.28f * temp);
                    if (gi >= 0 && gi < MapVisualizer.temperatureGradient.Length)
                    {
                        color = MapVisualizer.temperatureGradient[gi];
                    }
                }
                Brush brush = new SolidBrush(Color.FromArgb(128, color));
                Pen pen = new Pen(Color.FromArgb(192, Color.White), 2.0f);
                float baseRadius = selected ? 60.0f : 30.0f;
                float variation = selected ? 40.0f : 20.0f;
                float radius = baseRadius;
                float lx = xc, ly = yc;
                Point[] points = new Point[numPoints];

                for (int i = 0; i < numPoints + 1; i++)
                {
                    float x = xc, y = yc;
                    int index = (i % numPoints);

                    if (index < device.Samples.Count)
                    {
                        if (device.Samples[device.Samples.Count - 1 - index].valid)
                        {
                            radius = baseRadius + variation * ((device.Samples[device.Samples.Count - 1 - index].X - 512.0f) / 1024.0f);
                        }
                        x = xc + radius * (float)Math.Sin(index * 360.0f / numPoints / 180.0f * (float)Math.PI);
                        y = yc + radius * (float)-Math.Cos(index * 360.0f / numPoints / 180.0f * (float)Math.PI);
                    }

                    if (i > 0)
                    {
                        points[i - 1] = new Point((int)x, (int)y);
                    }

                    lx = x; ly = y;
                }

                g.FillPolygon(brush, points);
                g.DrawPolygon(pen, points);
            }

            // Draw inner region
            {
                Color color = Color.White;
                if (device.Samples.Count > 0 && device.Samples[device.Samples.Count - 1].valid)
                {
                    int light = device.Samples[device.Samples.Count - 1].Z;
                    int gi = (int)((light) / 4);
                    if (gi >= 0 && gi < MapVisualizer.lightGradient.Length)
                    {
                        color = MapVisualizer.lightGradient[gi];
                    }
                }
                Brush brush = new SolidBrush(Color.FromArgb(255, color));
                //Pen pen = new Pen(Color.FromArgb(192, Color.White), 1.0f);
                float baseRadius = selected ? 10.0f : 5.0f;
                float variation = selected ? 30.0f : 15.0f;
                float radius = baseRadius;
                float lx = xc, ly = yc;
                Point[] points = new Point[numPoints];

                for (int i = 0; i < numPoints + 1; i++)
                {
                    float x = xc, y = yc;
                    int index = (i % numPoints);

                    if (index < device.Samples.Count)
                    {
                        if (device.Samples[device.Samples.Count - 1 - index].valid)
                        {
                            radius = baseRadius + variation * ((device.Samples[device.Samples.Count - 1 - index].V) / 1024.0f);
                        }
                        x = xc + radius * (float)Math.Sin(index * 360.0f / numPoints / 180.0f * (float)Math.PI);
                        y = yc + radius * (float)-Math.Cos(index * 360.0f / numPoints / 180.0f * (float)Math.PI);
                    }

                    if (i > 0)
                    {
                        points[i - 1] = new Point((int)x, (int)y);
                    }

                    lx = x; ly = y;
                }

                g.FillPolygon(brush, points);
                //g.DrawPolygon(pen, points);
            }

        }

    }

    public class Router : Node
    {
        public ushort[] Neighbours { get; set; }

        public Router(int id, int x, int y, int a, string label)
            : base(id, x, y, a, label)
        {
            ;
            Neighbours = null;
        }

        public void Draw(Graphics g, bool selected)
        {
            float radius = selected ? 30.0f : 15.0f;
            Brush circleBrush = new SolidBrush(Color.FromArgb(128, Color.Blue));
            Pen circlePen = new Pen(Color.FromArgb(192, Color.LightBlue));

            g.FillEllipse(circleBrush, X - radius / 2, Y - radius / 2, radius, radius);
            g.DrawEllipse(circlePen, X - radius / 2, Y - radius / 2, radius, radius);

            Brush fontBrush = selected ? Brushes.White : Brushes.LightGray;
            Font font = new Font("Courier New", 10.0f);  // FontFamily.GenericMonospace, 

            string label = Label;
            if (label == null || label.Length == 0) { label = "R" + Id + ""; }

            SizeF labelSize = g.MeasureString(label, font);
            g.DrawString(label, font, fontBrush, X - labelSize.Width - radius, Y - font.Height / 2);
        }

    }

    public class Connection
    {
        public int A { get; protected set; }
        public int B { get; protected set; }
        private DateTime dataUpdate;
        public float DataAge { get { return (float)(DateTime.UtcNow - dataUpdate).TotalSeconds; } set { dataUpdate = DateTime.UtcNow - TimeSpan.FromSeconds(value); } }
        public bool Parent { get; protected set; }          // Parent ('B' parent of 'A'), otherwise neighbour

        public Connection(int a, int b, bool parent)
        {
            this.A = a;
            this.B = b;
            this.Parent = parent;
            DataAge = 0;
        }

    }

}
