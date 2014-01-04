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
        private PaletteCache _paletteCache = new PaletteCache("../VideoCache/");
        private List<Color> _currPalette = new List<Color>();

        private Image paletteImage, scrollImage;
        private Bitmap paletteBitmap, scrollBitmap, previewColorBitmap;
        //private Graphics g_scroll;
        private HSV hslPaletteColor;
        private Color paletteColor;
        private bool setLuminosity;

        private int paletteIndex; // index of palette for changing color
        private int interfaceHeight;
        private int paletteWidth;
        private const int buttonWidth = 40;
        private int videoHeight, videoWidth;

        // recoloring suggestions
        private List<ChoicePictureBox> suggestPicturesList = new List<ChoicePictureBox>();
        private List<Color> suggestPalette = new List<Color>();

        private bool useMouseOverPreview = true;

        DLLInterface _DLL;

        public VideoRecoloring(DLLInterface DLL)
        {
            _DLL = DLL;
            InitializeComponent();
            fpsBox.SelectedIndex = 1;
            videoHeight = 0; videoWidth = 0;
            panelDecision.Visible = false;

            paletteImage = Image.FromFile("../Data/palette.png");
            pictureBoxPalette.Image = paletteImage;
            paletteBitmap = new Bitmap(paletteImage.Width, paletteImage.Height);
            Graphics g = Graphics.FromImage(paletteBitmap);
            g.DrawImage(paletteImage, new Point(0, 0));

            scrollImage = Image.FromFile("../Data/scroll.png");
            pictureBoxScroll.Image = scrollImage;
            scrollBitmap = new Bitmap(scrollImage.Width, scrollImage.Height);
            //g_scroll = Graphics.FromImage(scrollBitmap);
            hslPaletteColor = new HSV(1.0, 1.0, 0.75);
            paletteColor = Util.HSLtoRGB(hslPaletteColor);
            RenderScroll(1.0, 1.0);
            setLuminosity = false;

            pictureBoxColor.Height = pictureBoxPalette.Bottom - videoBox.Bottom - openButton.Height - resetButton.Height - suggestButton.Height - 35;
            previewColorBitmap = new Bitmap(pictureBoxColor.Width, pictureBoxColor.Height);
            UpdatePreviewColor();

            pictureBoxScroll.Left = pictureBoxPalette.Right + 10;
            palettePanel.Left = pictureBoxScroll.Right + 10;
            interfaceHeight = paletteImage.Height;
            paletteWidth = videoBox.Right - palettePanel.Left;

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
                    videoHeight = _DLL.GetVideoHeight();
                    videoWidth = _DLL.GetVideoWidth();

                    videoBox.Height = videoHeight;
                    videoBox.Width = videoWidth;
                    int top = videoHeight + 30;
                    pictureBoxColor.Top = top;
                    openButton.Top = pictureBoxColor.Bottom + 10;
                    resetButton.Top = openButton.Bottom + 10;
                    suggestButton.Top = resetButton.Bottom + 10;
                    pictureBoxColor.Width = openButton.Width;
                    pictureBoxPalette.Top = top;
                    pictureBoxScroll.Top = top;
                    palettePanel.Top = top;
                    paletteWidth = videoWidth + 10 - palettePanel.Left;
                    this.Height = videoHeight + interfaceHeight + 100;
                    this.Width = videoWidth + 60;

                    for (int k = 0; k < _paletteSize; k++)
                    {
                        _currPalette.Add(Color.FromArgb(_DLL.GetVideoPalette(k, 0), _DLL.GetVideoPalette(k, 1), _DLL.GetVideoPalette(k, 2)));
                        _layerPreview.Add(false);
                    }
                    UpdatePaletteDisplay(true, true);
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
            UpdatePaletteDisplay(true, true);
        }

        private void saveButton_Click(object sender, EventArgs e)
        {

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
            Image frame = (Image)_DLL.GetBitmap("videoFrame");

            if (frame != null && (frame.Height != videoHeight || frame.Width != videoWidth))
            {
                videoBox.Image = (Image)(new Bitmap(frame, new Size(videoWidth, videoHeight)));
            }
            else
            {
                videoBox.Image = frame;
            }
            //videoBox.Image = (Image)_DLL.GetBitmap("videoFrame");
        }

        private void UpdateButtonSelection(int index) // highlight selected palette color
        {
            for (int i = 0; i < _paletteButtons.Count; i++)
            {
                if (i != index)
                {
                    _paletteButtons[i].FlatAppearance.BorderColor = Color.Black; // reset
                    _paletteButtons[i].FlatAppearance.BorderSize = 1;
                }
                else
                {
                    _paletteButtons[i].FlatAppearance.BorderColor = Color.Red;
                    _paletteButtons[i].FlatAppearance.BorderSize = 5;
                }
            }
            if (index > 0)
            {
                // update preview color
                paletteColor = _paletteButtons[index].BackColor;
                hslPaletteColor = Util.RGBtoHSL(paletteColor);
                UpdatePreviewColor();
                RenderScroll(hslPaletteColor.H, hslPaletteColor.S);
            }
        }

        private void UpdatePaletteDisplay(bool enableEvents = true, bool showOriginalPalette = true)
        {
            palettePanel.Controls.Clear();
            _paletteButtons.Clear();

            List<Color> showPalette;
            if (showOriginalPalette) showPalette = _currPalette;
            else showPalette = suggestPalette;

            int padding = 0;
            paletteWidth = videoBox.Right - palettePanel.Left;
            int buttonsPerRow = (int)(paletteWidth / buttonWidth);

            for (int l = 0; l < showPalette.Count; l++)
            {
                Color rgb = showPalette[l];

                Button element = new Button();
                element.BackColor = rgb;
                element.Width = buttonWidth;
                int Rows = (int)Math.Ceiling((float)showPalette.Count / (float)buttonsPerRow);
                element.Height = interfaceHeight / Rows;
                element.Left = (element.Width + padding) * (l % buttonsPerRow);
                element.Top = (element.Height + padding) * (l / buttonsPerRow);

                element.FlatStyle = FlatStyle.Flat;

                if (enableEvents)
                {
                    element.MouseUp += delegate(Object sender, System.Windows.Forms.MouseEventArgs e)
                    {
                        Button btn = ((Button)sender);
                        int index = _paletteButtons.IndexOf(btn);

                        if (e.Button == MouseButtons.Left)
                        {
                            if (!useMouseOverPreview && Control.ModifierKeys == Keys.Shift)
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
                                UpdateButtonSelection(index);
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
                    element.MouseHover += delegate(Object sender, EventArgs e)
                    {
                        if (useMouseOverPreview)
                        {
                            Button btn = (Button)sender;
                            int index = _paletteButtons.IndexOf(btn);
                            _DLL.SetVideoPreviewLayerIndex(index);
                        }
                    };
                    element.MouseLeave += delegate(Object sender, EventArgs e)
                    {
                        if (useMouseOverPreview)
                        {
                            _DLL.SetVideoPreviewLayerIndex(-1);
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
            double lum = hslPaletteColor.V;
            hslPaletteColor = new HSV(hue, saturation, lum);
            paletteColor = Util.HSLtoRGB(hslPaletteColor);
            UpdatePreviewColor();

            if (paletteIndex >= 0 && paletteIndex < _paletteButtons.Count)
            {
                _paletteButtons[paletteIndex].BackColor = paletteColor;
                _DLL.SetVideoPalette(paletteIndex, paletteColor.R, paletteColor.G, paletteColor.B);
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

        private void UpdatePreviewColor()
        {
            if (previewColorBitmap.Height < 1 || previewColorBitmap.Width < 1) return;
            // outline box
            for (int c = 0; c < previewColorBitmap.Width; c++)
            {
                previewColorBitmap.SetPixel(c, 0, Color.Black);
                previewColorBitmap.SetPixel(c, previewColorBitmap.Height - 1, Color.Black);
            }
            for (int r = 1; r < previewColorBitmap.Height - 1; r++)
            {
                previewColorBitmap.SetPixel(0, r, Color.Black);
                previewColorBitmap.SetPixel(previewColorBitmap.Width - 1, r, Color.Black);
            }
            // fill box
            for (int r = 1; r < previewColorBitmap.Height - 1; r++)
            {
                for (int c = 1; c < previewColorBitmap.Width - 1; c++)
                {
                    previewColorBitmap.SetPixel(c, r, paletteColor);
                }
            }
            pictureBoxColor.Image = previewColorBitmap;
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

                UpdatePreviewColor();
            }
        }

        private void pictureBoxScroll_MouseUp(object sender, MouseEventArgs e)
        {
            if (e.X >= 0 && e.X < scrollBitmap.Width && e.Y >= 0 && e.Y < scrollBitmap.Height)
            {
                if (setLuminosity)
                {
                    hslPaletteColor.V = 1d - (double)e.Y / scrollBitmap.Height;
                    paletteColor = Util.HSLtoRGB(hslPaletteColor);
                    UpdatePreviewColor();
                    if (paletteIndex >= 0 && paletteIndex < _paletteButtons.Count)
                    {
                        _paletteButtons[paletteIndex].BackColor = paletteColor;
                        _DLL.SetVideoPalette(paletteIndex, paletteColor.R, paletteColor.G, paletteColor.B);
                    }
                }
            }
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
                    UpdatePreviewColor();
                    if (paletteIndex >= 0 && paletteIndex < _paletteButtons.Count)
                    {
                        _paletteButtons[paletteIndex].BackColor = paletteColor;
                        _DLL.SetVideoPalette(paletteIndex, paletteColor.R, paletteColor.G, paletteColor.B);
                    }
                }
            }
        }

        private void suggestButton_Click(object sender, EventArgs e)
        {
            const int nrows = 3;
            const int ncols = 4;
            const int padding = 10;

            // look for cached files
            int imwidth = videoWidth / ncols - padding;
            int imheight = videoHeight / nrows - padding;

            int n = _DLL.LoadSuggestedRecolorings();
            if (n <= 0) return;
            n++; // include original image
            // display grid
            panelDecision.Visible = true;
            panelDecision.Left = videoBox.Left;
            panelDecision.Top = videoBox.Top;
            panelDecision.Height = (imheight + padding) * nrows;
            panelDecision.Width = (imwidth + padding) * ncols;
            // load
            panelDecision.Controls.Clear();
            suggestPicturesList.Clear();
            UpdateButtonSelection(-1);
            for (int i = 0; i < n; i++)
            {
                ChoicePictureBox p = new ChoicePictureBox(i); // (i / nrows, i % nrows) = (row, col)
                p.MouseUp += delegate(Object psender, System.Windows.Forms.MouseEventArgs pe)
                {
                    //ChoicePictureBox pic = ((ChoicePictureBox)sender);
                    //int index = suggestPicturesList.IndexOf(pic);
                    // load suggestion
                    _DLL.LoadSuggestion(p.choice);
                    _currPalette.Clear();
                    for (int k = 0; k < _paletteSize; k++)
                    {
                        _currPalette.Add(Color.FromArgb(_DLL.GetVideoPalette(k, 0), _DLL.GetVideoPalette(k, 1), _DLL.GetVideoPalette(k, 2)));
                    }
                    UpdatePaletteDisplay(true, true);
                    // return to display
                    panelDecision.Visible = false;
                    panelDecision.Controls.Clear();
                    suggestPicturesList.Clear();
                    suggestPalette.Clear();
                };
                p.MouseHover += delegate(Object psender, EventArgs pe)
                {
                    //ChoicePictureBox pic = ((ChoicePictureBox)sender);
                    //int index = suggestPicturesList.IndexOf(pic);
                    // show suggested palette
                    suggestPalette.Clear();
                    for (int k = 0; k < _paletteSize; k++)
                    {
                        suggestPalette.Add(Color.FromArgb(_DLL.GetSuggestPalette(p.choice, k, 0), _DLL.GetSuggestPalette(p.choice, k, 1), _DLL.GetSuggestPalette(p.choice, k, 2)));
                    }
                    UpdatePaletteDisplay(true, false);
                };
                p.MouseLeave += delegate(Object psender, EventArgs pe)
                {
                    UpdatePaletteDisplay(true, true); // show original palette
                };
                // layout
                p.Left = (i % ncols) * (imwidth + padding);
                p.Top = (i / ncols) * (imheight + padding);
                p.Height = imheight;
                p.Width = imwidth;

                Image suggestion = (Image)_DLL.GetBitmap("suggestFrame" + i);
                if (suggestion.Height != imheight || suggestion.Width != imwidth)
                {
                    suggestion = (Image)(new Bitmap(suggestion, new Size(imwidth, imheight)));
                }
                p.Image = suggestion;
                //p.Image = (Image)_DLL.GetBitmap("suggestFrame" + i);
                panelDecision.Controls.Add(p);
                suggestPicturesList.Add(p);
            }
        }
    }
}