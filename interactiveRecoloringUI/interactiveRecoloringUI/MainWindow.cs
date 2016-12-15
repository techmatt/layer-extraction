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
        const int brushEllipseRadius = 3;
        const string stagingDir = @"C:\Code\layer-extraction\interactiveRecoloringUI\staging\";

        string imageFullPath;
        string imageName;
        Bitmap originalImage;
        Bitmap activeImage;
        Bitmap paletteImage;
        Bitmap editImage;
        Bitmap resultImage;
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
            resultImage = new Bitmap(originalImage);
            activeImageGraphics = Graphics.FromImage(activeImage);
            editImageGraphics = Graphics.FromImage(editImage);

            editImageGraphics.Clear(Color.FromArgb(0, 0, 0, 0));

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
        
        private void drawEditCircle(MouseEventArgs e)
        {
            Color c = selectedColor;
            if (e.Button == MouseButtons.Right)
                c = Color.Magenta;

            Brush brush = new SolidBrush(c);
            int radius = brushEllipseRadius;
            if (Control.ModifierKeys == Keys.Control)
            {
                brush = new SolidBrush(Color.FromArgb(0, 0, 0, 0));
                radius *= 2;
            }

            editImageGraphics.CompositingMode = System.Drawing.Drawing2D.CompositingMode.SourceCopy;
            editImageGraphics.FillEllipse(brush, e.X - radius, e.Y - radius, radius * 2, radius * 2);
            editImageGraphics.CompositingMode = System.Drawing.Drawing2D.CompositingMode.SourceOver;

            updateEditBox();
        }

        private void pictureBoxMain_MouseDown(object sender, MouseEventArgs e)
        {
            if (Control.ModifierKeys == Keys.Shift)
            {
                updateSelectedColor(activeImage.GetPixel(e.Location.X, e.Location.Y));
                return;
            }
            drawEditCircle(e);
        }

        private void pictureBoxMain_MouseMove(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left || e.Button == MouseButtons.Right)
            {
                drawEditCircle(e);
            }
        }

        private void updateResultBox()
        {
            pictureBoxResult.Image = resultImage;
        }
        private void updateEditBox()
        {
            if(checkBoxOriginal.Checked)
                activeImageGraphics.DrawImageUnscaled(originalImage, new Point(0, 0));
            else
                activeImageGraphics.DrawImageUnscaled(resultImage, new Point(0, 0));
            if(checkBoxShowConstraints.Checked)
                activeImageGraphics.DrawImageUnscaled(editImage, new Point(0, 0));
            pictureBoxMain.Image = activeImage;
        }
        private void update()
        {
            originalImage.Save(stagingDir + imageName + ".png");

            activeImageGraphics.DrawImageUnscaled(originalImage, new Point(0, 0));
            activeImageGraphics.DrawImageUnscaled(editImage, new Point(0, 0));
            activeImage.Save(stagingDir + imageName + "-edits.png");

            var lines = new List<string>();
            lines.Add("inputImage=" + stagingDir + imageName + ".png");
            lines.Add("editImage=" + stagingDir + imageName + "-edits.png");
            File.WriteAllLines(stagingDir + "recoloringParamsFromUI.txt", lines);
            File.WriteAllText(stagingDir + "update.txt", "update");
            while (true)
            {
                if (!File.Exists(stagingDir + "update.txt"))
                    break;
                Thread.Sleep(50);
            }
            resultImage = loadImageFromFile(stagingDir + imageName + "-result.png");
            updateResultBox();
            updateEditBox();
        }

        private void buttonUpdate_Click(object sender, EventArgs e)
        {
            update();
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

        private void label3_Click(object sender, EventArgs e)
        {

        }

        private void checkBoxOriginal_CheckedChanged(object sender, EventArgs e)
        {
            updateResultBox();
            updateEditBox();
        }

        private void checkBoxShowConstraints_CheckedChanged(object sender, EventArgs e)
        {
            updateResultBox();
            updateEditBox();
        }

        private void buttonSave_Click(object sender, EventArgs e)
        {
            Bitmap saveImage = new Bitmap(originalImage, originalImage.Width * 3, originalImage.Height);
            Graphics g = Graphics.FromImage(saveImage);
            g.DrawImageUnscaled(originalImage, new Point(originalImage.Width * 0, 0));
            g.DrawImageUnscaled(originalImage, new Point(originalImage.Width * 1, 0));
            g.DrawImageUnscaled(editImage, new Point(originalImage.Width * 1, 0));
            g.DrawImageUnscaled(resultImage, new Point(originalImage.Width * 2, 0));
            saveImage.Save(stagingDir + "save.png");
        }
    }
}
