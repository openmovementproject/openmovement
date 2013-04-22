using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Drawing.Drawing2D;
using OmApiNet;
using System.Threading;

namespace OmGui
{
    public partial class DataViewer : UserControl
    {
        public DataViewer()
        {
            InitializeComponent();
            //this.MouseWheel += DataViewer_MouseWheel;
            Close();
            Mode = ModeType.Zoom;
        }

        public enum ModeType { Selection, Zoom };
        public ModeType mode;
        public ModeType Mode
        {
            get { return mode; }
            set
            {
                mode = value;
                toolStripButtonSelection.Checked = (mode == ModeType.Selection);
                toolStripButtonZoom.Checked = (mode == ModeType.Zoom);
            }
        }
        private void toolStripButtonSelection_Click(object sender, EventArgs e) { Mode = ModeType.Selection; }
        private void toolStripButtonZoom_Click(object sender, EventArgs e)  { Mode = ModeType.Zoom; }


        private BackgroundWorker backgroundWorkerPreview;
        private AutoResetEvent backgroundWorkerPreviewStopped = new AutoResetEvent(false);

        private OmReader reader;
        public OmReader Reader
        {
            get { return reader; }
            set
            {
                // Stop existing preview
                CancelPreview();

                // Close existing
                Close();

                animate = 0.0f;
                bitmapDirty = true;

                reader = value;
                if (reader != null)
                {
                    dataBlockCache.Set(reader.DataNumBlocks);
                    timerAnimate.Enabled = true;
                }
                else
                {
                    dataBlockCache.Set(0);
                    timerAnimate.Enabled = false;
                    //Refresh();
                }

                firstBlock = 0;
                numBlocks = 0;
                if (reader != null) { numBlocks = reader.DataNumBlocks; }
                offsetBlocks = 0;
                if (reader != null) { offsetBlocks = reader.DataOffsetBlocks; }
                beginBlock = endBlock = 0.0f;

                StartPreview();
                bitmapDirty = true;
//                Refresh();
            }
        }

        public void Close()
        {
            CancelPreview();
            if (reader != null)
            {
                Reader.Close();
            }
            reader = null;
            bitmapDirty = true;
        }

        public void Open(ushort deviceId)
        {
            Reader = OmReader.Open(deviceId);
        }

        public void Open(string filename)
        {
            Reader = OmReader.Open(filename);
        }

        bool bitmapDirty = true;
        Bitmap myBitmap = null;

        float firstBlock = 0.0f;
        float numBlocks = 0.0f;

        int offsetBlocks = 0;
        public int OffsetBlocks { get { return offsetBlocks; } }

        // Animate: 0 = use live values, 1 = (start animation) use 'animate from' values, interpolated in-between
        DateTime lastAnimateTime = DateTime.MinValue;
        float animate = 0.0f;
        float animateFirstBlock = 0.0f;
        float animateNumBlocks = 0.0f;

        DataBlockCache dataBlockCache = new DataBlockCache();

        public new void Refresh()
        {
            bitmapDirty = true;
            if (endBlock == beginBlock)
            {
                graphPanel.SetSelection(-1, -1, "");
            }
            else
            {
                graphPanel.SetSelection(PointForBlock(beginBlock), PointForBlock(endBlock), TimeForBlock(beginBlock) + " - " + TimeForBlock(endBlock));
            }
            Invalidate();
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            try
            {
                if (bitmapDirty)
                {
                    bool displayX = checkBoxX.Checked, displayY = checkBoxY.Checked, displayZ = checkBoxZ.Checked;
                    bool displayAccel = checkBoxAccel.Checked;
                    bool displayLight = checkBoxLight.Checked, displayTemp = checkBoxTemp.Checked, displayBatt = checkBoxBatt.Checked;

                    int width = graphPanel.Width;
                    if (myBitmap == null || myBitmap.Width != width || myBitmap.Height != graphPanel.Height)
                    {
                        myBitmap = new Bitmap(width, graphPanel.Height, System.Drawing.Imaging.PixelFormat.Format24bppRgb);
                    }

                    Pen penMissing = new Pen(Brushes.LightGray);
                    Pen penMissing2 = new Pen(Brushes.LightGray);
                    Pen penLine = new Pen(Brushes.LightGray);
                    Pen penUnprocessed = new Pen(Brushes.LightGray);
                    Pen penCurrent = new Pen(Brushes.Black);
                    Pen penEdge = new Pen(Brushes.DarkGray);
                    Pen penDataX = new Pen(Color.FromArgb(96, Color.Red));
                    Pen penDataY = new Pen(Color.FromArgb(96, Color.Green));
                    Pen penDataZ = new Pen(Color.FromArgb(96, Color.Blue));
                    Pen penDataAccel = new Pen(Color.FromArgb(96, Color.Black));
                    Pen penDataLight = new Pen(Color.FromArgb(96, Color.Brown));
                    Pen penDataTemp = new Pen(Color.FromArgb(96, Color.DarkMagenta));
                    Pen penDataBatt = new Pen(Color.FromArgb(96, Color.DarkCyan));

                    // Fade-zoom
                    //int fadedA = (int)(255 * (1.0f - animate));
                    //if (fadedA < 0) { fadedA = 0; }
                    //if (fadedA > 255) { fadedA = 255; }
                    //Pen penFaded = new Pen(Color.FromArgb(fadedA, Color.Black));
                    //float trueFirstX = PointForBlock(firstBlock);
                    //float trueLastX = PointForBlock(firstBlock + numBlocks);

                    Graphics g = Graphics.FromImage(myBitmap);
                    g.Clear(Color.LightGray);

                    if (reader != null)
                    {
                        //int drawBlock = -1;
                        for (int x = 0; x < myBitmap.Width; x++)
                        {
                            float block = BlockAtPoint(x);
                            float block2 = BlockAtPoint(x + 1);
                            float tolerance;

                            //DataBlock dataBlock = dataBlockCache.PeekDataBlock(block);
                            Aggregate aggregate = dataBlockCache.GetAggregate(block, block2, out tolerance);

                            if (aggregate.present)
                            {
                                /*
                                if (block == lastUpdateBlock)
                                {
                                    g.DrawLine(penCurrent, x, 0, x, 0 + myBitmap.Height - 1);
                                }
                                else if (!blockData[block].processed)
                                {
                                    g.DrawLine(penUnprocessed, x, 0, x, 0 + myBitmap.Height - 1);
                                }
                                else if (blockData[block].boundary)
                                {
                                    g.DrawLine(penEdge, x, 0, x, 0 + myBitmap.Height - 1);
                                }
                                else
                                {
                                    g.DrawLine(penLine, x, 0, x, 0 + myBitmap.Height - 1);
                                }
                                */

                                float center = 0.5f;
                                float scale = 0.10f;

                                // Axis
                                if ((x & 3) < 2) g.DrawRectangle(penMissing2, x, (center + scale * 0.0f) * myBitmap.Height, 1, 1);

                                if (displayX) g.DrawRectangle(penDataX, x, (center + scale * aggregate.Min.X) * myBitmap.Height, 1, 1 + ((scale * (aggregate.Max.X - aggregate.Min.X))) * myBitmap.Height);
                                if (displayY) g.DrawRectangle(penDataY, x, (center + scale * aggregate.Min.Y) * myBitmap.Height, 1, 1 + ((scale * (aggregate.Max.Y - aggregate.Min.Y))) * myBitmap.Height);
                                if (displayZ) g.DrawRectangle(penDataZ, x, (center + scale * aggregate.Min.Z) * myBitmap.Height, 1, 1 + ((scale * (aggregate.Max.Z - aggregate.Min.Z))) * myBitmap.Height);

                                //if (displayAccel) g.DrawRectangle(penDataAccel, x, (center + scale * aggregate.Min.Amplitude) * myBitmap.Height, 1, 1 + ((scale * (aggregate.Max.Amplitude - aggregate.Min.Amplitude))) * myBitmap.Height);

                                if (displayLight) { float height = ((aggregate.Max.Light - aggregate.Min.Light) / 1024.0f);     g.DrawRectangle(penDataLight, x, (1.0f - (height + aggregate.Min.Light / 1024.0f)) * myBitmap.Height, 1, 1 + height * myBitmap.Height); }
                                if (displayTemp) { float height = -0.02f * (aggregate.Max.Temp - aggregate.Min.Temp) / 1000.0f; g.DrawRectangle(penDataTemp,  x, (1.0f - (height + 0.02f * aggregate.Min.Temp / 1000.0f)) * myBitmap.Height, 1, 1 + height * myBitmap.Height); }
                                if (displayBatt) { float height = (aggregate.Max.Batt - aggregate.Min.Batt + 1) / 102.0f;       g.DrawRectangle(penDataBatt,  x, (1.0f - (height + (aggregate.Min.Batt + 1) / 102.0f)) * myBitmap.Height, 1, 1 + height * myBitmap.Height); }
                            }
                            else
                            {
                                g.DrawLine(((x >> 2) & 1) == 0 ? penMissing : penMissing2, x, 0, x, 0 + myBitmap.Height - 1);
                            }

                            //if (x < trueFirstX || x >= trueLastX)
                            //{
                            //    g.DrawLine(penFaded, x, 0, x, 0 + myBitmap.Height - 1);
                            //}
                        }

                        /*
                        int downloadX = downloadProgress * myBitmap.Width / 100;

                        ColorBlend progressColorBlend = new ColorBlend(3);
                        progressColorBlend.Colors = new Color[] { Color.FromArgb(0x66, Color.Green), Color.FromArgb(0xcc, Color.White), Color.FromArgb(0x66, Color.Green) };
                        progressColorBlend.Positions = new float[] { 0.0f, 0.3f, 1.0f };

                        LinearGradientBrush progressBrush = new LinearGradientBrush(new Rectangle(0, 0, 1, myBitmap.Height - 1), Color.Black, Color.Black, 90.0f);
                        progressBrush.InterpolationColors = progressColorBlend;
                        g.FillRectangle(progressBrush, 0, 0, downloadX, myBitmap.Height);

                        ColorBlend backColorBlend = new ColorBlend(3);
                        backColorBlend.Colors = new Color[] { Color.FromArgb(0x33, Color.DarkGray), Color.FromArgb(0x66, Color.WhiteSmoke), Color.FromArgb(0x33, Color.DarkGray) };
                        backColorBlend.Positions = new float[] { 0.0f, 0.3f, 1.0f };

                        LinearGradientBrush backBrush = new LinearGradientBrush(new Rectangle(0, 0, 1, myBitmap.Height - 1), Color.Black, Color.Black, 90.0f);
                        backBrush.InterpolationColors = backColorBlend;
                        g.FillRectangle(backBrush, downloadX, 0, myBitmap.Width - downloadX, myBitmap.Height);
                        */

                    }

                    //g.DrawString("" + width, Font, new SolidBrush(ForeColor), new PointF(50, 50));

                    graphPanel.Image = myBitmap;
                }
                bitmapDirty = false;
            }
            catch (Exception) { ; }

            base.OnPaint(e);
        }

        private void graphPanel_SizeChanged(object sender, EventArgs e)
        {
            Refresh();
        }

        private void backgroundWorkerPreview_DoWork(object sender, DoWorkEventArgs e)
        {
            try
            {
                DateTime lastUpdate = DateTime.MinValue;
                int width = graphPanel.Width;
                if (width <= 0) { return; }
                for (int step = 64; step > 0; step /= 2)
                {
                    if (backgroundWorkerPreview.CancellationPending) { break; }
                    for (int x = 0; x < width; x += step)
                    {
                        if (backgroundWorkerPreview.CancellationPending) { break; }

                        int blockNumber = (int)BlockAtPoint(x);


//float tolerance;
//Aggregate aggregate = dataBlockCache.GetAggregate(block, block2, out tolerance);


                        dataBlockCache.FetchDataBlock(reader, blockNumber);

                        DateTime now = DateTime.Now;
                        if (lastUpdate == DateTime.MinValue || now - lastUpdate > TimeSpan.FromMilliseconds(250))
                        {
                            Refresh();
                            lastUpdate = now;
                        }

//Thread.Sleep(10);
                    }
                }
                Refresh();
            }
            finally
            {
                backgroundWorkerPreviewStopped.Set();
            }
        }

        public void StartPreview()
        {
//            Refresh();
            if (backgroundWorkerPreview == null && reader != null)
            {
                backgroundWorkerPreviewStopped.Reset();
                backgroundWorkerPreview = new BackgroundWorker();
                backgroundWorkerPreview.WorkerSupportsCancellation = true;
                backgroundWorkerPreview.DoWork += backgroundWorkerPreview_DoWork;
                backgroundWorkerPreview.RunWorkerAsync();
            }
        }

        public void CancelPreview()
        {
            if (backgroundWorkerPreview != null)
            { 
                backgroundWorkerPreview.CancelAsync();
                backgroundWorkerPreviewStopped.WaitOne();
                backgroundWorkerPreview = null;
                backgroundWorkerPreviewStopped.Reset();
            }
        }

        private void checkBox_CheckedChanged(object sender, EventArgs e)
        {
            Refresh();
        }

        public void StartAnimation(float newFirstBlock, float newNumBlocks, bool preview)
        {
            // Record current values as start of animation
            animateFirstBlock = firstBlock;
            animateNumBlocks = numBlocks;
            animate = 1.0f;
            lastAnimateTime = DateTime.UtcNow;

            // Set new values
            firstBlock = newFirstBlock;
            numBlocks = newNumBlocks;

            // Check ranges and clip
            if (numBlocks > reader.DataNumBlocks) { numBlocks = reader.DataNumBlocks; }
            if (numBlocks < 1) { numBlocks = 1; }
            if (firstBlock + numBlocks > reader.DataNumBlocks) { firstBlock = reader.DataNumBlocks - numBlocks; }
            if (firstBlock < 0) { firstBlock = 0; }

            graphPanel_MouseMove(null, null);        // Update display
            Refresh();

            if (preview)
            {
                CancelPreview();
                StartPreview();
            }
        }

        public bool HasSelection { get { return beginBlock != endBlock; } }
        public float BlockAtPoint(float x, bool afterAnimation = false)
        {
            float aFirstBlock = firstBlock;
            float aNumBlocks = numBlocks;
            if (!afterAnimation && animate > 0)
            {
                aFirstBlock = animate * animateFirstBlock + (1.0f - animate) * firstBlock;
                aNumBlocks = animate * animateNumBlocks + (1.0f - animate) * numBlocks;
            }

            if (reader == null) { return 0; }
            if (graphPanel.Width == 0) { return 0; }
            return (float)x * aNumBlocks / graphPanel.Width + aFirstBlock;
        }

        public float PointForBlock(float block, bool afterAnimation = false)
        {
            float aFirstBlock = firstBlock;
            float aNumBlocks = numBlocks;
            if (!afterAnimation && animate > 0)
            {
                aFirstBlock = animate * animateFirstBlock + (1.0f - animate) * firstBlock;
                aNumBlocks = animate * animateNumBlocks + (1.0f - animate) * numBlocks;
            }

            if (reader == null) { return 0; }
            if (aNumBlocks == 0) { return 0; }
            return (float)graphPanel.Width * (block - aFirstBlock) / aNumBlocks;
        }

        public DateTime TimeForBlock(float block)
        {
            float tolerance;
            Aggregate aggregate = dataBlockCache.GetAggregate(block, block, out tolerance);
            return aggregate.Min.T;
        }

        public string TimeString(DateTime time)
        {
            if (time == DateTime.MinValue) { return ""; }
            return String.Format("{0:yyyy-MM-dd HH:mm:ss}", time);
        }

        float beginBlock = 0;
        float endBlock = 0;

        public float SelectionBeginBlock { get { return beginBlock; } }
        public float SelectionEndBlock { get { return endBlock; } }

        public enum SelectionType { SelectionNone, SelectionMove, SelectionBegin, SelectionEnd };
        private SelectionType selectionType = SelectionType.SelectionNone;

        public SelectionType GetSelectionTypeAtPoint(float x)
        {
            // If no selection...
            if (!HasSelection)
            {
                return SelectionType.SelectionNone;
            }

            // Find the position of the begin and end markers
            float beginBlockX = PointForBlock(beginBlock, true);
            float endBlockX = PointForBlock(endBlock, true);
            float margin = 5;

            // Check if within a graspable margin
            bool isBegin = Math.Abs(x - beginBlockX) <= margin;
            bool isEnd = Math.Abs(x - endBlockX) <= margin;

            // If close to both, return closest
            if (isBegin && isEnd)
            {
                return (Math.Abs(x - beginBlockX) <= Math.Abs(x - endBlockX)) ? SelectionType.SelectionBegin : SelectionType.SelectionEnd;
            }

            // Start or end marker
            if (isBegin) { return SelectionType.SelectionBegin; }
            if (isEnd) { return SelectionType.SelectionEnd; }

            // In-between markers
            if ((x >= beginBlockX && x <= endBlockX) || (x >= endBlockX && x <= beginBlockX))
            {
                return SelectionType.SelectionMove;
            }

            // Outside selection
            return SelectionType.SelectionNone;
        }

        float mouseOffsetX = 0;

        private void graphPanel_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == System.Windows.Forms.MouseButtons.Left)
            {
                SelectionType over = GetSelectionTypeAtPoint(e.X);
                if (over == SelectionType.SelectionNone && Mode == ModeType.Selection)
                {
                    selectionType = SelectionType.SelectionEnd;
                    graphPanel.Cursor = Cursors.IBeam;
                    beginBlock = BlockAtPoint(e.X);
                    endBlock = beginBlock;
                    mouseOffsetX = 0;
                }
                else if (over == SelectionType.SelectionBegin)
                {
                    selectionType = SelectionType.SelectionBegin;
                    mouseOffsetX = e.X - PointForBlock(beginBlock);
                }
                else if (over == SelectionType.SelectionEnd)
                {
                    selectionType = SelectionType.SelectionEnd;
                    mouseOffsetX = e.X - PointForBlock(endBlock);
                }
                else if (over == SelectionType.SelectionMove && Mode == ModeType.Selection)
                {
                    selectionType = SelectionType.SelectionMove;
                    mouseOffsetX = e.X - PointForBlock(beginBlock);
                }

                graphPanel.Capture = true;
                graphPanel_MouseMove(sender, e);        // Update display
            }
        }

        private void graphPanel_MouseUp(object sender, MouseEventArgs e)
        {
            if (e.Button == System.Windows.Forms.MouseButtons.Left)
            {
                graphPanel.Capture = false;
                selectionType = SelectionType.SelectionNone;
                if (reader != null)
                {
                    if (beginBlock < 0.0f) { beginBlock = 0.0f; }
                    if (beginBlock > reader.DataNumBlocks) { beginBlock = reader.DataNumBlocks; }
                    if (endBlock < 0.0f) { endBlock = 0.0f; }
                    if (endBlock > reader.DataNumBlocks) { endBlock = reader.DataNumBlocks; }
                }
                graphPanel_MouseMove(sender, e);        // Update cursor type and display
            }
        }

        //private ToolTip toolTip = new ToolTip();
        private Point lastLocation = new Point(int.MinValue, int.MinValue);

        private void graphPanel_MouseLeave(object sender, EventArgs e)
        {
            lastLocation = new Point(int.MinValue, int.MinValue);
            graphPanel.SetCursor(-1.0f, "");
        }

        private void graphPanel_MouseMove(object sender, MouseEventArgs e)
        {
            if (e != null)
            {
                float blockAtCursor = BlockAtPoint(e.X);

                if (e.Location != lastLocation)
                {
                    lastLocation = e.Location;

                    graphPanel.SetCursor(e.X, TimeString(TimeForBlock(blockAtCursor)));

                    /*
                    if (aggregate.Min.T > DateTime.MinValue)
                    {
                        toolTip.SetToolTip(graphPanel, String.Format("{0:yyyy-MM-dd HH:mm:ss}", aggregate.Min.T));
                        toolTip.IsBalloon = true;
                        toolTip.ReshowDelay = 250;
                        toolTip.Active = true;
                    }
                    else
                    {
                        toolTip.Active = false;
                    }
                    */
                }

                // Operation preview cursor
                if (selectionType == SelectionType.SelectionNone)
                {
                    SelectionType over = GetSelectionTypeAtPoint(e.X);
                    if (over == SelectionType.SelectionNone && Mode == ModeType.Selection){ graphPanel.Cursor = Cursors.IBeam; }
                    else if (over == SelectionType.SelectionBegin) { graphPanel.Cursor = Cursors.SizeWE; }
                    else if (over == SelectionType.SelectionEnd) { graphPanel.Cursor = Cursors.SizeWE; }
                    else if (over == SelectionType.SelectionMove && Mode == ModeType.Selection) { graphPanel.Cursor = Cursors.SizeAll; }
                    else 
                    {
                        using (System.IO.MemoryStream ms = new System.IO.MemoryStream(OmGui.Properties.Resources.zoom))
                        {
                            graphPanel.Cursor = new Cursor(ms);
                        }
                    }

                    return;
                }

                // Selection manipulation
                if (selectionType == SelectionType.SelectionBegin)
                {
                    beginBlock = BlockAtPoint(e.X - mouseOffsetX);
                    if (beginBlock > endBlock)
                    {
                        float t = endBlock; endBlock = beginBlock; beginBlock = t;    // swap
                        selectionType = SelectionType.SelectionEnd;
                    }
                }
                else if (selectionType == SelectionType.SelectionEnd)
                {
                    endBlock = BlockAtPoint(e.X - mouseOffsetX);
                    if (beginBlock > endBlock)
                    {
                        float t = endBlock; endBlock = beginBlock; beginBlock = t;    // swap
                        selectionType = SelectionType.SelectionBegin;
                    }
                }
                else if (selectionType == SelectionType.SelectionMove)
                {
                    float span = endBlock - beginBlock;
                    beginBlock = BlockAtPoint(e.X - mouseOffsetX);
                    endBlock = beginBlock + span;
                }
            }

            if (endBlock == beginBlock)
            {
                graphPanel.SetSelection(-1, -1, "");
            }
            else
            {
                graphPanel.SetSelection(PointForBlock(beginBlock), PointForBlock(endBlock), TimeForBlock(beginBlock) + " - " + TimeForBlock(endBlock));
            }
        }

        /*
        public void DataViewer_MouseWheel(object sender, MouseEventArgs e)
        {
            if (reader != null && selectionType == SelectionType.SelectionNone && reader != null && numBlocks > 0)
            {
                float oldNumBlocks = numBlocks;

                if (e.Delta < 0)
                {
                    numBlocks = numBlocks / 2;
                }
                else if (e.Delta > 0)
                {
                    numBlocks = numBlocks * 2;
                }

                if (beginBlock < 0.0f) { beginBlock = 0.0f; }
                if (numBlocks < 1) { numBlocks = 1; }
                if (beginBlock + numBlocks > reader.DataNumBlocks) { numBlocks = reader.DataNumBlocks - beginBlock; }
                if (numBlocks < 1) { numBlocks = 1; }
                if (beginBlock + numBlocks > reader.DataNumBlocks) { beginBlock = reader.DataNumBlocks - numBlocks; }
                if (beginBlock < 0.0f) { beginBlock = 0.0f; }

                float midBlock = BlockAtPoint(e.X);
                beginBlock = (oldNumBlocks / 2) * (numBlocks / oldNumBlocks);

                if (beginBlock < 0.0f) { beginBlock = 0.0f; }
                if (beginBlock + numBlocks > reader.DataNumBlocks) { numBlocks = reader.DataNumBlocks - beginBlock; }
                if (numBlocks < 1) { numBlocks = 1; }
                if (beginBlock + numBlocks > reader.DataNumBlocks) { beginBlock = reader.DataNumBlocks - numBlocks; }
                if (beginBlock < 0.0f) { beginBlock = 0.0f; }

            }
            graphPanel_MouseMove(sender, e);        // Update cursor type and display
        }
        */

        private void ZoomIn(float a)
        {
            if (numBlocks < 2) { return; }
            float newNumBlocks = numBlocks / 2;
            float newFirstBlock = a - (newNumBlocks / 2);
            StartAnimation(newFirstBlock, newNumBlocks, true);
        }

        private void ZoomOut(float a)
        {
            float newNumBlocks = numBlocks * 2;
            float newFirstBlock = a - (newNumBlocks / 2);
            StartAnimation(newFirstBlock, newNumBlocks, true);
        }

        private void ZoomRange(float a, float b)
        {
            float newFirstBlock = a;
            float newNumBlocks = b - a;
            StartAnimation(newFirstBlock, newNumBlocks, true);
        }

        private void graphPanel_DoubleClick(object sender, EventArgs ea)
        {
            MouseEventArgs e = (MouseEventArgs)ea;
            if (e.Button == System.Windows.Forms.MouseButtons.Left && HasSelection && GetSelectionTypeAtPoint(e.X) != SelectionType.SelectionNone)
            {
                selectionType = SelectionType.SelectionNone;
                graphPanel.Capture = false;
                ZoomRange(beginBlock, endBlock);
            }
        }

        private void graphPanel_Paint(object sender, PaintEventArgs e)
        {

        }

        private void graphPanel_Click(object sender, EventArgs ea)
        {
            MouseEventArgs e = (MouseEventArgs)ea;
            if (Mode == ModeType.Zoom && e.Button == System.Windows.Forms.MouseButtons.Left && selectionType == SelectionType.SelectionNone && reader != null && reader.DataNumBlocks > 0)
            {
                ZoomIn(BlockAtPoint(e.X));
            }
            if (Mode == ModeType.Zoom && e.Button == System.Windows.Forms.MouseButtons.Right && selectionType == SelectionType.SelectionNone && reader != null && reader.DataNumBlocks > 0)
            {
                ZoomOut(BlockAtPoint(e.X));
            }
            if (Mode == ModeType.Selection && e.Button == System.Windows.Forms.MouseButtons.Right && selectionType == SelectionType.SelectionNone)
            {
                beginBlock = endBlock = 0;
                graphPanel_MouseMove(sender, e);
                StartAnimation(firstBlock, numBlocks, false);
            }

        }


        private void timerAnimate_Tick(object sender, EventArgs e)
        {
            if (animate > 0.0f)
            {
                DateTime now = DateTime.UtcNow;
                float elapsed = (float)(now - lastAnimateTime).TotalSeconds;
                lastAnimateTime = now;
                if (elapsed < 0) { elapsed = 1; }
                animate -= elapsed * 3.0f;
                if (animate < 0.0f) { animate = 0.0f; }
                bitmapDirty = true;
            }
            if (bitmapDirty) { Refresh(); }
        }

    }


    public class GraphPanel : Panel
    {
        public GraphPanel() : base()
        {
            this.DoubleBuffered = true;
        }

        private Bitmap image;
        public Bitmap Image
        {
            set { this.image = value; Invalidate(); }
            get { return image; }
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);
            if (image != null)
            {
                e.Graphics.DrawImage(image, ClientRectangle);
            }
            if (selectionEnd != selectionStart)
            {
                Rectangle rect = new Rectangle((int)selectionStart, 0, (int)(selectionEnd - selectionStart), this.Height - 1);
                e.Graphics.FillRectangle(new SolidBrush(Color.FromArgb(0x40, SystemColors.Highlight)), rect);
                e.Graphics.DrawRectangle(SystemPens.Highlight, rect);

                string s = selectionLabel;
                Font font = SystemFonts.DefaultFont;
                Size stringSize = e.Graphics.MeasureString(s, font).ToSize();
                Point point = new Point((int)((selectionStart + selectionEnd) / 2 - stringSize.Width / 2), 16);
                e.Graphics.FillRectangle(new SolidBrush(Color.FromArgb(0x64, Color.FromKnownColor(KnownColor.Info))), new RectangleF(point, stringSize));
                e.Graphics.DrawRectangle(Pens.Black, new Rectangle(point, stringSize));
                e.Graphics.DrawString(s, font, SystemBrushes.InfoText, point);
            }
            if (cursorPos >= 0)
            {
                Pen pen = new Pen(Color.DarkGray, 1.0f); pen.DashStyle = DashStyle.Dot;
                e.Graphics.DrawLine(pen, cursorPos, 0.0f, cursorPos, this.Height);

                string s = cursorLabel;
                Font font = SystemFonts.DefaultFont;
                Size stringSize = e.Graphics.MeasureString(s, font).ToSize();
                Point point = new Point((int)(cursorPos - stringSize.Width / 2), this.Height - stringSize.Height - 5);
                e.Graphics.FillRectangle(new SolidBrush(Color.FromArgb(0x64, Color.FromKnownColor(KnownColor.Info))), new RectangleF(point, stringSize));
                e.Graphics.DrawRectangle(Pens.Black, new Rectangle(point, stringSize));
                e.Graphics.DrawString(s, font, SystemBrushes.InfoText, point);
            }
//e.Graphics.DrawString("" + this.ClientRectangle.Width, Font, new SolidBrush(ForeColor), ClientRectangle);
        }

        private float selectionStart = 0.0f, selectionEnd = 0.0f;
        private string selectionLabel = "";

        private float cursorPos = -1.0f;
        private string cursorLabel = "";

        public void SetSelection(float start, float end, string label)
        {
            selectionStart = start;
            selectionEnd = end;
            selectionLabel = label;
            Invalidate();
        }

        public void SetCursor(float pos, string label)
        {
            cursorPos = pos;
            cursorLabel = label;
            Invalidate();
        }
    }


    public struct Sample
    {
        public Sample(DateTime t, float x, float y, float z, float light, float temp, float batt) : this() { T = t; X = x; Y = y; Z = z; Light = light; Temp = temp; Batt = batt; }
        public DateTime T { get; set; }
        public float X { get; set; }
        public float Y { get; set; }
        public float Z { get; set; }
        public float Light { get; set; }
        public float Temp { get; set; }
        public float Batt { get; set; }
        public float Amplitude { get { return (float)Math.Sqrt(X * X + Y * Y + Z * Z); } }
        public static Sample Zero = new Sample(DateTime.MinValue, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        /*
        public static Sample Minimum(Sample a, Sample b)
        {
            if (a.X <= b.X && a.Y <= b.Y && a.Z <= b.Z) { return a; }
            if (b.X <= a.X && b.Y <= a.Y && b.Z <= a.Z) { return b; }
            return new Sample(Math.Min(a.X, b.X), Math.Min(a.Y, b.Y), Math.Min(a.Z, b.Z));
        }
        public static Sample Maximum(Sample a, Sample b)
        {
            if (a.X >= b.X && a.Y >= b.Y && a.Z >= b.Z) { return a; }
            if (b.X >= a.X && b.Y >= a.Y && b.Z >= a.Z) { return b; }
            return new Sample(Math.Max(a.X, b.X), Math.Max(a.Y, b.Y), Math.Max(a.Z, b.Z));
        }
        */
        public static Sample Interpolate(Sample a, Sample b, float p)
        {
            DateTime t = new DateTime((long)((1.0f - p) * (double)a.T.Ticks + p * (double)b.T.Ticks));
            float x = (1.0f - p) * a.X + p * b.X;
            float y = (1.0f - p) * a.Y + p * b.Y;
            float z = (1.0f - p) * a.Z + p * b.Z;
            float light = (1.0f - p) * a.Light + p * b.Light;
            float temp = (1.0f - p) * a.Temp + p * b.Temp;
            float batt = (1.0f - p) * a.Batt + p * b.Batt;
            return new Sample(t, x, y, z, light, temp, batt);
        }
    }

    public struct Aggregate
    {
        public bool present;
        public Sample Min;
        public Sample Max;
        public static readonly Aggregate Zero = new Aggregate();
        
        public void Add(Sample sample)
        {
            if (!present)
            {
                Min = sample;
                Max = sample;
                present = true;
                return;
            }

            if (sample.T != DateTime.MinValue && sample.T < Min.T) { Min.T = sample.T; } if (sample.T > Max.T) { Max.T = sample.T; }
            if (sample.X < Min.X) { Min.X = sample.X; } if (sample.X > Max.X) { Max.X = sample.X; }
            if (sample.Y < Min.Y) { Min.Y = sample.Y; } if (sample.Y > Max.Y) { Max.Y = sample.Y; }
            if (sample.Z < Min.Z) { Min.Z = sample.Z; } if (sample.Z > Max.Z) { Max.Z = sample.Z; }
            if (sample.Light < Min.Light) { Min.Light = sample.Light; } if (sample.Light > Max.Light) { Max.Light = sample.Light; }
            if (sample.Temp < Min.Temp) { Min.Temp = sample.Temp; } if (sample.Temp > Max.Temp) { Max.Temp = sample.Temp; }
            if (sample.Batt < Min.Batt) { Min.Batt = sample.Batt; } if (sample.Batt > Max.Batt) { Max.Batt = sample.Batt; }
            //Min = Sample.Minimum(Min, sample);
            //Max = Sample.Maximum(Max, sample);
        }

        public void Add(Sample[] samples)
        {
            foreach (Sample sample in samples)
            {
                Add(sample);
            }
        }

        public void Add(Aggregate aggregate)
        {
            Add(aggregate.Min);
            Add(aggregate.Max);
        }


    }

    public class DataBlock
    {
        public short[] RawValues { get; protected set; }
        public Sample[] Values { get; protected set; }
        private Aggregate aggregate;
        public Aggregate Aggregate { get { return aggregate; } }

        public DataBlock(DateTime firstTime, DateTime lastTime, float light, float temp, float batt, short[] raw)
        {
            RawValues = raw;
            Values = new Sample[raw.Length / 3];
            double spanMilliseconds = (lastTime - firstTime).TotalMilliseconds;
            for (int i = 0; i < Values.Length; i++)
            {
                DateTime t;
                if (Values.Length <= 1) { t = firstTime; }
                else { t = firstTime.AddMilliseconds(spanMilliseconds * i / (Values.Length - 1)); }
                float x = raw[3 * i + 0] / 256.0f;
                float y = raw[3 * i + 1] / 256.0f;
                float z = raw[3 * i + 2] / 256.0f;
                Values[i] = new Sample(t, x, y, z, light, temp, batt);
            }
            aggregate.Add(Values);
        }

        public static DataBlock FromReader(OmReader reader, int blockNumber)
        {
            try
            {
                reader.Seek(blockNumber);
                short[] values = reader.ReadBlock();
                if (values == null)
                {
                    return null;
                }
                DateTime firstTime = reader.TimeForSample(0);
                DateTime lastTime;
                float light = reader.Light;
                float temp = reader.Temp;
                float batt = reader.Batt;
                if (values.Length == 0) { lastTime = firstTime; }
                else { lastTime = reader.TimeForSample(values.Length - 1); }
                return new DataBlock(firstTime, lastTime, light, temp, batt, values);
            }
            catch (Exception)
            {
                return null;
            }
        }

        public Sample InterpolatedValue(float index)
        {
            int i = (int)index;
            float p = index - i;
            if (i < 0) { i = 0; p = 0.0f; }
            if (i >= Values.Length - 1) { i = Values.Length - 1; p = 0.0f; }
            if (Values.Length == 0) { return Sample.Zero; }
            if (p == 0.0f) { return Values[i]; }
            return Sample.Interpolate(Values[i], Values[i + 1], p);
        }
    }

    public class DataBlockCache
    {
        private int cacheSize = 10 * 2 * 1024;

        LinkedList<int> lru = new LinkedList<int>();
        IDictionary<int, LinkedListNode<int>> lruNodes = new Dictionary<int, LinkedListNode<int>>();
        SortedList<int, DataBlock> cache = new SortedList<int, DataBlock>();

        //int levels;
        //Aggregate[] aggregates;

        public DataBlockCache()
        {
            Set(0);
        }

        public void Set(int numBlocks)
        {
            lock (this)
            {
                lru.Clear();
                lruNodes.Clear();
                cache.Clear();
                //if (numBlocks == 0)
                //{
                //    levels = 0;
                //    aggregates = null;
                //} 
                //else
                //{
                //    levels = (int)Math.Log(numBlocks, 2);
                //    aggregates = new Aggregate[1 << (levels + 1)];
                //}
            }
        }

        public Aggregate GetAggregate(float blockStart, float blockEnd, out float tolerance)
        {
            //int i = ((1 << (levels - 1)) - 1) + blockNumber;
            //for (; ; )
            //{
            //    Aggregate aggregate = aggregates[i];
            //    if (aggregate.present) { return aggregate; }
            //    if (i == 0) { break; }
            //    i = ((i - 1) / 2);
            //}
            //return Aggregate.Zero;

            int blockNumber = (int)blockStart;

            lock (this)
            {
                if (cache.ContainsKey(blockNumber))
                {
                    tolerance = 0;
                    DataBlock dataBlock = cache[blockNumber];
                    if (dataBlock == null) { return Aggregate.Zero; }
                    if ((blockEnd - blockStart) > 0.25f || dataBlock.Values.Length <= 0)
                    {
                        return dataBlock.Aggregate;
                    }
                    else
                    {
                        int startSample = (int)((blockStart - (int)blockStart) * dataBlock.Values.Length);
                        int endSample = dataBlock.Values.Length - 1;
                        if ((int)blockStart == (int)blockEnd) { endSample = (int)((blockEnd - (int)blockEnd) * dataBlock.Values.Length); }
                        if (endSample < startSample) { endSample = startSample; }
                        Aggregate a = Aggregate.Zero;
                        for (int i = startSample; i <= endSample; i++)
                        {
                            a.Add(dataBlock.Values[i]);
                        }
                        return a;
                    }
                }
                else
                {
                    int result = cache.Keys.BinarySearch(blockNumber);
                    if (result < 0) { result = ~result; }
                    if (result >= cache.Keys.Count) { result--; }
                    if (result < 0 || result >= cache.Keys.Count) { tolerance = 0; return Aggregate.Zero; }
                    tolerance = Math.Abs(result - blockNumber);
                    KeyValuePair<int, DataBlock> d = cache.ElementAt(result);
                    if (d.Value == null) { return Aggregate.Zero; }
                    return d.Value.Aggregate;
                }
            }

        }

        public DataBlock PeekDataBlock(int blockNumber)
        {
            DataBlock dataBlock = FetchDataBlock(null, blockNumber);
            if (dataBlock == null) { return null; }
            return dataBlock;
        }

        public DataBlock FetchDataBlock(OmReader reader, int blockNumber)
        {
            lock (this)
            {
                if (cache.ContainsKey(blockNumber))
                {
                    // Find block in cache
                    LinkedListNode<int> node = lruNodes[blockNumber];

                    // Move to end of LRU
                    lru.Remove(node);
                    lru.AddLast(node);

                    // Return block
                    return cache[blockNumber];
                }
            }

            if (reader == null)
            {
                return null;
            }

            // Read block
            DataBlock dataBlock = DataBlock.FromReader(reader, blockNumber);

            lock (this)
            {
                // Add to cache
                cache[blockNumber] = dataBlock;
                LinkedListNode<int> node = new LinkedListNode<int>(blockNumber);
                lruNodes[blockNumber] = node;
                lru.AddLast(node);

                // Trim cache to maximum size
                while (lru.Count > cacheSize)
                {
                    int evict = lru.First.Value;
                    lru.RemoveFirst();
                    lruNodes.Remove(evict);
                    cache.Remove(evict);
                }

                //// Push in to aggregates
                //int i = ((1 << (levels - 1)) - 1) + blockNumber;
                //for (; ; )
                //{
                //    aggregates[i].Add(dataBlock.Aggregate);
                //    if (i == 0) { break; }
                //    i = ((i - 1) / 2);
                //}

                // Return block
                return dataBlock;
            }
        }

    }

}
