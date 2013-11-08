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
        private BackgroundWorker bw = new BackgroundWorker();
        private List<Button> palette = new List<Button>();
        private String currVideoFile = "";
        private float FPS = 24;
        private int paletteSize = 5;
        private PaletteCache paletteCache = new PaletteCache("../VideoCache/");
        private List<Color> currPalette = new List<Color>();

        public VideoRecoloring()
        {
            InitializeComponent();
        }

        private void openButton_Click(object sender, EventArgs e)
        {
            ClearPalette();


            OpenFileDialog dialog = new OpenFileDialog();
            dialog.Filter = "Text Files (*.txt)|*.txt";
            dialog.ShowDialog();

            if (dialog.FileName != "")
            {
                String filepath = dialog.FileName;
                // parse file

                /*activeImage = Image.FromFile(filepath);
                pictureBox.Image = activeImage;
                pictureBoxBitmap = new Bitmap(pictureBox.Image.Width, pictureBox.Image.Height);
                g = Graphics.FromImage(pictureBoxBitmap);
                currImageFile = filepath;

                String basename = new FileInfo(filepath).Name;
                if (paletteCache.Exists("turk", 5, basename) && !paletteMethodBox.Items.Contains("TurkAverage"))
                {
                    paletteMethodBox.Items.Add("TurkAverage");
                }
                else if (paletteMethodBox.Items.Contains("TurkAverage"))
                    paletteMethodBox.Items.Remove("TurkAverage");*/

            }
        }

        private void ClearPalette()
        {
            currPalette.Clear();
            palette.Clear();
            palettePanel.Controls.Clear();
            previewBox.Image = new Bitmap(100, 100);
            //pictureBoxOriginal.Image = new Bitmap(100, 100);
            //layers = new Layers();
        }
    }
}



//            BCProcessCommand(baseCodeDLLContext, "ExtractVideoLayers");