using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.IO;
using System.Net;
using System.Linq;
using System.Diagnostics;
using System.Windows.Input;
using System.Threading;

namespace interactiveRecoloringUI
{
    public partial class MainWindow : Form
    {
        const int brushEllipseRadius = 4;
        const string stagingDir = @"C:\Code\layer-extraction\interactiveRecoloringUI\staging\";

        string imageFullPath;
        string imageName;
        Bitmap originalImage;
        Bitmap activeImage;
        Bitmap paletteImage;
        Bitmap editImage;
        Graphics editImageGraphics;
        Graphics activeImageGraphics;
        Color selectedColor;
        
        Bitmap loadImageFromFile(string filename)
        {
            Bitmap result;
            using (var bmpTemp = new Bitmap(filename))
            {
                result = new Bitmap(bmpTemp);
            }
            return result;
        }

        public MainWindow()
        {
            InitializeComponent();
            paletteImage = new Bitmap(pictureBoxPalette.Image);
            loadImage(@"C:\Code\layer-extraction\interactiveRecoloring\examples\faceB-tiny.png");
        }

        private void pictureBoxPalette_Click(object sender, EventArgs e)
        {

        }

        private void buttonLoadImage_Click(object sender, EventArgs e)
        {
            OpenFileDialog dialog = new OpenFileDialog();
            dialog.Filter = "Image Files (*.png)|*.png";
            dialog.ShowDialog();

            if (dialog.FileName != "")
            {
                loadImage(dialog.FileName);
            }
        }

        private void loadImage(string filename)
        {
            imageFullPath = filename;
            originalImage = loadImageFromFile(filename);
            activeImage = new Bitmap(originalImage);
            editImage = new Bitmap(originalImage);
            activeImageGraphics = Graphics.FromImage(activeImage);
            editImageGraphics = Graphics.FromImage(editImage);

            pictureBoxMain.Image = activeImage;
            imageName = Path.GetFileNameWithoutExtension(filename);

            pictureBoxResult.Top = pictureBoxMain.Bottom + 20;
        }

        private void updateSelectedColor(Color newColor)
        {
            selectedColor = newColor;
            pictureBoxSelectedColor.BackColor = selectedColor;
        }

        private void pictureBoxPalette_MouseDown(object sender, MouseEventArgs e)
        {
            updateSelectedColor(paletteImage.GetPixel(e.Location.X, e.Location.Y));
        }

        private void MainWindow_Load(object sender, EventArgs e)
        {

        }

        private void pictureBoxMain_MouseDown(object sender, MouseEventArgs e)
        {
            if (Control.ModifierKeys == Keys.Shift)
            {
                updateSelectedColor(activeImage.GetPixel(e.Location.X, e.Location.Y));
                return;
            }
            Color c = selectedColor;
            if (e.Button == MouseButtons.Right)
                c = Color.Magenta;
            activeImageGraphics.FillEllipse(new SolidBrush(c), e.X - brushEllipseRadius, e.Y - brushEllipseRadius, brushEllipseRadius * 2, brushEllipseRadius * 2);
            pictureBoxMain.Image = activeImage;
        }

        private void pictureBoxMain_MouseMove(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left || e.Button == MouseButtons.Right)
            {
                Color c = selectedColor;
                if (e.Button == MouseButtons.Right)
                    c = Color.Magenta;
                activeImageGraphics.FillEllipse(new SolidBrush(c), e.X - brushEllipseRadius, e.Y - brushEllipseRadius, brushEllipseRadius * 2, brushEllipseRadius * 2);
                pictureBoxMain.Image = activeImage;
            }
        }

        private void buttonUpdate_Click(object sender, EventArgs e)
        {
            originalImage.Save(stagingDir + imageName + ".png");
            activeImage.Save(stagingDir + imageName + "-edits.png");

            var lines = new List<string>();
            lines.Add("inputImage=" + stagingDir + imageName + ".png");
            lines.Add("editImage=" + stagingDir + imageName + "-edits.png");
            File.WriteAllLines(stagingDir + "recoloringParamsFromUI.txt", lines);
            File.WriteAllText(stagingDir + "update.txt", "update");
            while(true)
            {
                if (!File.Exists(stagingDir + "update.txt"))
                    break;
                Thread.Sleep(50);
            }
            pictureBoxResult.Image = loadImageFromFile(stagingDir + imageName + "-result.png");
        }

        private void buttonReset_Click(object sender, EventArgs e)
        {
            loadImage(imageFullPath);
        }

        private void pictureBoxMain_Click(object sender, EventArgs e)
        {

        }

        private void pictureBoxSelectedColor_Click(object sender, EventArgs e)
        {
            colorDialog.Color = selectedColor;
            colorDialog.FullOpen = true;
            DialogResult result = colorDialog.ShowDialog();
            if(result == DialogResult.OK)
            {
                updateSelectedColor(colorDialog.Color);
            }
        }
    }
}
