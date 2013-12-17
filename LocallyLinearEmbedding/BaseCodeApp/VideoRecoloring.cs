using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Engine;

namespace BaseCodeApp
{
    public partial class VideoRecoloring : Form
    {
        private BackgroundWorker _bw = new BackgroundWorker();
        private List<Button> _paletteButtons = new List<Button>();
        private List<bool> _layerPreview = new List<bool>();
        private String _currVideoFile = "";
        private float[] _FPS = { 5, 8, 9, 10, 12.5f, 15, 24 };
        private int _paletteSize = 5;
        private int _paletteButtonWidth = 9;
        private PaletteCache _paletteCache = new PaletteCache("../VideoCache/");
        private List<Color> _currPalette = new List<Color>();

        private Image paletteImage, scrollImage;
        private Bitmap paletteBitmap, scrollBitmap;
        private Graphics g_scroll;
        private HSV hslPaletteColor;
        private Color paletteColor;
        private bool setLuminosity;

        private int paletteIndex; // index of palette for changing color
        private int interfaceHeight;

        DLLInterface _DLL;

        public VideoRecoloring(DLLInterface DLL)
        {
            _DLL = DLL;
            InitializeComponent();
            fpsBox.SelectedIndex = 1;


            paletteImage = Image.FromFile("../Data/palette.png");
            pictureBoxPalette.Image = paletteImage;
            paletteBitmap = new Bitmap(paletteImage.Width, paletteImage.Height);
            Graphics g = Graphics.FromImage(paletteBitmap);
            g.DrawImage(paletteImage, new Point(0, 0));

            scrollImage = Image.FromFile("../Data/scroll.png");
            pictureBoxScroll.Image = scrollImage;
            scrollBitmap = new Bitmap(scrollImage.Width, scrollImage.Height);
            g_scroll = Graphics.FromImage(scrollBitmap);
            hslPaletteColor = new HSV(1.0, 1.0, 0.5);
            RenderScroll(1.0, 1.0);
            setLuminosity = false;

            pictureBoxScroll.Left = pictureBoxPalette.Right + 10;
            palettePanel.Left = pictureBoxScroll.Right + 10;
            interfaceHeight = paletteImage.Height;

            paletteIndex = -1;
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
                    _paletteSize = _DLL.GetVideoPaletteSize(); // may have changed if using user-defined palette
                    KBox.Text = _paletteSize.ToString();
                };
                _bw.RunWorkerCompleted += delegate
                {
                    for (int k = 0; k < _paletteSize; k++)
                    {
                        _currPalette.Add(Color.FromArgb(_DLL.GetVideoPalette(k, 0), _DLL.GetVideoPalette(k, 1), _DLL.GetVideoPalette(k, 2)));
                        _layerPreview.Add(false);
                    }
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
                _layerPreview[i] = false;
            }
            UpdatePaletteDisplay(true);
        }

        private void saveButton_Click(object sender, EventArgs e)
        {
            if (_bw.IsBusy) return;
            _bw = new BackgroundWorker();
            _bw.DoWork += delegate
            {
                _DLL.SaveVideoFrames();
            };
            _bw.RunWorkerAsync();
        }

        private void savePaletteImageButton_Click(object sender, EventArgs e)
        {
            if (_bw.IsBusy) return;
            _bw = new BackgroundWorker();
            _bw.DoWork += delegate
            {
                _DLL.SaveVideoPaletteImage();
            };
            _bw.RunWorkerAsync();
        }

        private void fpsBox_Changed(object sender, EventArgs e)
        {
            int idx = fpsBox.SelectedIndex;
            timerVideoFrame.Interval = (int)(1000 / _FPS[idx]);
        }

        private void ClearPalette()
        {
            _currPalette.Clear();
            _paletteButtons.Clear();
            _layerPreview.Clear();
            palettePanel.Controls.Clear();
        }

        private void timerVideoFrame_Tick(object sender, EventArgs e)
        {
            Image im = _DLL.GetBitmap("videoFrame");
            videoBox.Image = im; //(Image)_DLL.GetBitmap("videoFrame");
            if (im != null)
            {
                videoBox.Height = im.Height;
                videoBox.Width = im.Width;
                int top = videoBox.Bottom + 20;
                openButton.Top = top;
                resetButton.Top = openButton.Bottom + 10;
                saveVideoButton.Top = resetButton.Bottom + 10;
                pictureBoxPalette.Top = top;
                pictureBoxScroll.Top = top;
                palettePanel.Top = top;

                this.Height = videoBox.Height + interfaceHeight + 90;
                this.Width = videoBox.Width + 50;
            }
        }

        private void UpdatePaletteDisplay(bool enableEvents = true)
        {
            palettePanel.Controls.Clear();
            _paletteButtons.Clear();

            int padding = 10;

            for (int l = 0; l < _currPalette.Count; l++)
            {
                Color rgb = _currPalette[l];

                Button element = new Button();
                element.BackColor = rgb;
                element.Width = 40;
                element.Height = interfaceHeight;
                if (l < _paletteButtonWidth)
                    element.Left = element.Width * l + padding;
                else
                {
                    element.Left = element.Width * (l - _paletteButtonWidth) + padding;
                    element.Top = element.Height + padding;
                }

                element.FlatStyle = FlatStyle.Flat;

                if (enableEvents)
                {
                    element.MouseUp += delegate(Object sender, System.Windows.Forms.MouseEventArgs e)
                    {
                        Button btn = ((Button)sender);
                        int index = _paletteButtons.IndexOf(btn);


                        if (e.Button == MouseButtons.Left)
                        {
                            if (Control.ModifierKeys == Keys.Shift)
                            {
                                // layer preview
                                if (!_layerPreview[index])
                                    _DLL.SetVideoPreviewLayerIndex(index);
                                else
                                    _DLL.SetVideoPreviewLayerIndex(-1);
                                _layerPreview[index] = !_layerPreview[index];
                            }
                            else
                            {
                                paletteIndex = index;
                                _layerPreview[index] = false;
                                /*Open the color picker and recolor
                                colorPicker.Color = btn.BackColor;
                                colorPicker.ShowDialog();

                                if (colorPicker.Color != btn.BackColor)
                                {
                                    btn.BackColor = colorPicker.Color;

                                    _DLL.SetVideoPalette(index, colorPicker.Color.R, colorPicker.Color.G, colorPicker.Color.B);
                                    _layerPreview[index] = false;
                                }*/
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

        private void VideoRecoloring_Load(object sender, EventArgs e)
        {

        }

        private void pictureBoxPalette_Click(object sender, EventArgs e)
        {

        }

        private void pictureBoxPalette_MouseDown(object sender, MouseEventArgs e)
        {
            Color rgb = paletteBitmap.GetPixel(e.X, e.Y);
            //this.BackColor = Color.FromArgb(rgb.R, rgb.G, rgb.B);
            double hue = (double)rgb.GetHue() / 360.0;
            double saturation = (double)rgb.GetSaturation();

            RenderScroll(hue, saturation);
            hslPaletteColor = new HSV(hue, saturation, 0.5);
            paletteColor = Util.HSLtoRGB(hslPaletteColor);

            if (paletteIndex > 0 && paletteIndex < _paletteButtons.Count)
            {
                _paletteButtons[paletteIndex].BackColor = paletteColor;
                _DLL.SetVideoPalette(paletteIndex, paletteColor.R, paletteColor.G, paletteColor.B);

                //this.BackColor = paletteColor;
            }
        }

        private void RenderScroll(double hue, double saturation)
        {
            int h = scrollBitmap.Height;
            float increment = 1f / scrollBitmap.Height;
            for (int r = 0; r < scrollBitmap.Height; r++)
            {
                for (int c = 0; c < scrollBitmap.Width; c++)
                {
                    HSV pcolor = new HSV(hue, saturation, increment * r);
                    scrollBitmap.SetPixel(c, scrollBitmap.Height - r - 1, Util.HSLtoRGB(pcolor));
                }
            }
            pictureBoxScroll.Image = scrollBitmap;
        }

        private void pictureBoxScroll_Click(object sender, EventArgs e)
        {

        }

        private void pictureBoxScroll_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Y >= 0 && e.Y < scrollBitmap.Height)
            {
                setLuminosity = true;
                hslPaletteColor.V = 1d - (double)e.Y / scrollBitmap.Height;
                paletteColor = Util.HSLtoRGB(hslPaletteColor);

                //this.BackColor = paletteColor;
            }
        }

        private void pictureBoxScroll_MouseUp(object sender, MouseEventArgs e)
        {
            setLuminosity = false;
        }

        private void pictureBoxScroll_MouseMove(object sender, MouseEventArgs e)
        {
            if (e.X >= 0 && e.X < scrollBitmap.Width && e.Y >= 0 && e.Y < scrollBitmap.Height)
            {
                if (setLuminosity)
                {
                    hslPaletteColor.V = 1d - (double)e.Y / scrollBitmap.Height;
                    paletteColor = Util.HSLtoRGB(hslPaletteColor);
                    if (paletteIndex > 0 && paletteIndex < _paletteButtons.Count)
                    {
                        _paletteButtons[paletteIndex].BackColor = paletteColor;
                        _DLL.SetVideoPalette(paletteIndex, paletteColor.R, paletteColor.G, paletteColor.B);

                        //this.BackColor = paletteColor;
                    }
                }
            }
        }
    }
}