using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace BaseCodeApp
{
    public partial class VideoRecoloring : Form
    {
        //private Layers layers = new Layers();
        private BackgroundWorker _bw = new BackgroundWorker();
        private List<Button> _paletteButtons = new List<Button>();
        private String _currVideoFile = "";
        private float _FPS = 24;
        private int _paletteSize = 5;
        private PaletteCache _paletteCache = new PaletteCache("../VideoCache/");
        private List<Color> _currPalette = new List<Color>();

        DLLInterface _DLL;

        public VideoRecoloring(DLLInterface DLL)
        {
            _DLL = DLL;
            InitializeComponent();
        }

        private void openButton_Click(object sender, EventArgs e)
        {
            if (_bw.IsBusy) return;

            ClearPalette();

            String defaultparamsfile = "../VideoParameters.txt";
            if (!System.IO.File.Exists(defaultparamsfile))
            {
                OpenFileDialog dialog = new OpenFileDialog();
                dialog.Filter = "Text Files (*.txt)|*.txt";
                dialog.ShowDialog();

                if (dialog.FileName != "") _currVideoFile = dialog.FileName;
            }
            else
            {
                _currVideoFile = defaultparamsfile;
            }

            _paletteSize = Int32.Parse(KBox.Text);
            if (_paletteSize <= 0) _paletteSize = 5;

            if (_currVideoFile != "")
            {
                _bw = new BackgroundWorker();
                _bw.DoWork += delegate
                {
                    _DLL.LoadVideo(_currVideoFile, _paletteSize);
                };
                _bw.RunWorkerCompleted += delegate
                {
                    for (int k = 0; k < _paletteSize; k++)
                        _currPalette.Add(Color.FromArgb(_DLL.GetVideoPalette(k, 0), _DLL.GetVideoPalette(k, 1), _DLL.GetVideoPalette(k, 2)));
                    UpdatePaletteDisplay(true);
                    this.Cursor = System.Windows.Forms.Cursors.Default;
                };
                _bw.RunWorkerAsync();
            }
        }

        private void resetButton_Click(object sender, EventArgs e)
        {
            // reset the palette (which resets the video)
            for (int i = 0; i < _currPalette.Count; i++)
            {
                _currPalette[i] = Color.FromArgb(_DLL.GetOriginalVideoPalette(i, 0), _DLL.GetOriginalVideoPalette(i, 1), _DLL.GetOriginalVideoPalette(i, 2));
                _DLL.SetVideoPalette(i, _currPalette[i].R, _currPalette[i].G, _currPalette[i].B);
            }
            UpdatePaletteDisplay(true);
        }

        private void ClearPalette()
        {
            _currPalette.Clear();
            _paletteButtons.Clear();
            palettePanel.Controls.Clear();
            previewBox.Image = new Bitmap(100, 100);
            //pictureBoxOriginal.Image = new Bitmap(100, 100);
            //layers = new Layers();
        }

        private void timerVideoFrame_Tick(object sender, EventArgs e)
        {
            videoBox.Image = (Image)_DLL.GetBitmap("videoFrame");
        }

        private void UpdatePaletteDisplay(bool enableEvents = true)
        {
            Console.WriteLine("updating palette display");

            palettePanel.Controls.Clear();
            _paletteButtons.Clear();

            int padding = 10;

            for (int l = 0; l < _currPalette.Count; l++)
            {
                Color rgb = _currPalette[l];

                Button element = new Button();
                element.BackColor = rgb;
                element.Width = 40;
                element.Height = 150;
                element.Left = element.Width * l + padding;
                element.FlatStyle = FlatStyle.Flat;

                if (enableEvents)
                {
                    element.MouseUp += delegate(Object sender, System.Windows.Forms.MouseEventArgs e)
                    {
                        //Open the color picker and recolor
                        Button btn = ((Button)sender);
                        int index = _paletteButtons.IndexOf(btn);
                        if (e.Button == MouseButtons.Left)
                        {
                            colorPicker.Color = btn.BackColor;
                            colorPicker.ShowDialog();

                            if (colorPicker.Color != btn.BackColor)
                            {
                                btn.BackColor = colorPicker.Color;

                                _DLL.SetVideoPalette(index, colorPicker.Color.R, colorPicker.Color.G, colorPicker.Color.B);
                                /*pictureBox.Image = Recolor(layers, palette.Select(c => new DenseVector(new double[] { c.BackColor.R, c.BackColor.G, c.BackColor.B })).ToList<DenseVector>(), ColorSpace.RGB);*/
                            }
                        }
                    };
                }

                _paletteButtons.Add(element);
                palettePanel.Controls.Add(element);
            }

            /*count convex hull
            if (currPalette.colors.Count > 3)
            {
                statusBox.Text = "Convex hull error: " + WithinConvexHull(space);
            }*/

        }
    }
}