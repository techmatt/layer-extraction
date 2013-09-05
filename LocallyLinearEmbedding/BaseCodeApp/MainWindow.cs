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
using MathNet.Numerics.LinearAlgebra.Double;
using Engine;
using System.Linq;
using System.Diagnostics;

namespace BaseCodeApp
{
    public partial class MainWindow : Form
    {
        [StructLayout(LayoutKind.Sequential), Serializable]
        public struct BCBitmapInfo
        {
            [MarshalAs(UnmanagedType.U4)]
            public int width;
            [MarshalAs(UnmanagedType.U4)]
            public int height;
            [MarshalAs(UnmanagedType.SysInt)]
            public IntPtr colorData;
        }

        [StructLayout(LayoutKind.Sequential), Serializable]
        public struct BCLayerInfo
        {
            [MarshalAs(UnmanagedType.U4)]
            public int width;
            [MarshalAs(UnmanagedType.U4)]
            public int height;
            [MarshalAs(UnmanagedType.R8)]
            public double d0;
            [MarshalAs(UnmanagedType.R8)]
            public double d1;
            [MarshalAs(UnmanagedType.R8)]
            public double d2;
            [MarshalAs(UnmanagedType.SysInt)]
            public IntPtr weights;
        };

        [StructLayout(LayoutKind.Sequential), Serializable]
        struct BCLayers
        {
            [MarshalAs(UnmanagedType.U4)]
            public int numLayers;
            [MarshalAs(UnmanagedType.SysInt)]
            public IntPtr layers;
        };



        const string BaseCodeDLL = "BaseCode.dll";
        [DllImport(BaseCodeDLL)]
        private static extern IntPtr BCInit();
        [DllImport(BaseCodeDLL)]
        private static extern UInt32 BCProcessCommand(IntPtr context, [In, MarshalAs(UnmanagedType.LPStr)] String command);
        [DllImport(BaseCodeDLL)]
        private static extern IntPtr BCQueryBitmapByName(IntPtr context, [In, MarshalAs(UnmanagedType.LPStr)] String bitmapName);
        [DllImport(BaseCodeDLL)]
        private static extern IntPtr BCQueryStringByName(IntPtr context, [In, MarshalAs(UnmanagedType.LPStr)] String stringName);
        [DllImport(BaseCodeDLL)]
        private static extern Int32 BCQueryIntegerByName(IntPtr context, [In, MarshalAs(UnmanagedType.LPStr)] String integerName);
        [DllImport(BaseCodeDLL)]
        private static extern IntPtr BCExtractLayers(IntPtr context, BCBitmapInfo bitmap, IntPtr palette, [In, MarshalAs(UnmanagedType.I4)]int paletteSize);
        [DllImport(BaseCodeDLL)]
        private static extern IntPtr BCSegmentImage(IntPtr context, BCBitmapInfo bitmap);


        IntPtr baseCodeDLLContext = (IntPtr)0;

        public enum ColorSpace
        {
            RGB, LAB
        }

        public class Layers
        {
            public List<double[,]> layers = new List<double[,]>();
            public int width;
            public int height;
            public List<DenseVector> colors = new List<DenseVector>();
            public ColorSpace space = ColorSpace.RGB;
            public List<Bitmap> previews = new List<Bitmap>();
        }

        private Layers layers = new Layers();
        private BackgroundWorker bw = new BackgroundWorker();
        private List<Button> palette = new List<Button>();
        private String currImageFile = "";
        private int CHITrials = 5;
        private PaletteCache paletteCache = new PaletteCache("../PaletteCache");
        private PaletteData currPalette = new PaletteData();

        public MainWindow()
        {
            InitializeComponent();

            if (baseCodeDLLContext == (IntPtr)0)
            {
                baseCodeDLLContext = BCInit();
            }

            //UpdateImages();
            paletteMethodBox.SelectedIndex = 1;
            layerMethodBox.SelectedIndex = 0;

        }

        private void SaveSaliencyMap(String dir, String key)
        {
            if (File.Exists(Path.Combine(dir, "saliency", Util.ConvertFileName(key, "_Judd")))) return;


            Directory.CreateDirectory(Path.Combine(dir, "saliency"));
            String exeDir = new DirectoryInfo("../../SaliencyExe").FullName;
            ProcessStartInfo info = new ProcessStartInfo();
            info.FileName = Path.Combine(exeDir, "Saliency.exe");
            info.WorkingDirectory = exeDir;

            String fullDir = new DirectoryInfo(dir).FullName;
            info.Arguments = "\""+Path.Combine(fullDir,key)+"\"";

            Process process = new Process();
            process.StartInfo = info;

            try
            {

               process.Start();
               process.WaitForExit();
            }
            catch (Exception e)
            {
                Console.WriteLine("Could not extract saliency map");
                Console.WriteLine(e.StackTrace);
            }

        }

        private void SaveSegmentation(String dir, String key)
        {

            if (File.Exists(Path.Combine(dir, "segments", key))) return;

            Bitmap bmp = new Bitmap(Path.Combine(dir, key));
            Directory.CreateDirectory(Path.Combine(dir, "segments"));

            Color[] bmpData = Util.BitmapTo1DArray(bmp);

            //for now, pass in RGB format
            byte[] rgbData = new byte[bmp.Width * bmp.Height * 3];
            for (int idx = 0; idx < bmpData.Length; idx++)
            {
                Color color = bmpData[idx];
                rgbData[3 * idx] = (byte)color.R;
                rgbData[3 * idx + 1] = (byte)color.G;
                rgbData[3 * idx + 2] = (byte)color.B;
            }


            BCBitmapInfo bmpInfo = new BCBitmapInfo();
            bmpInfo.colorData = Marshal.AllocHGlobal(Marshal.SizeOf(rgbData[0]) * rgbData.Length);
            bmpInfo.width = bmp.Width;
            bmpInfo.height = bmp.Height;

            try
            {
                Marshal.Copy(rgbData, 0, bmpInfo.colorData, rgbData.Length);
            }
            finally
            {
            }

            IntPtr bitmapInfoUnmanaged = BCSegmentImage(baseCodeDLLContext, bmpInfo);
            Marshal.FreeHGlobal(bmpInfo.colorData);

            if (bitmapInfoUnmanaged == (IntPtr)0) return;

            BCBitmapInfo bitmapInfo = (BCBitmapInfo)Marshal.PtrToStructure(bitmapInfoUnmanaged, typeof(BCBitmapInfo));

            //return new Bitmap(bitmapInfo.width, bitmapInfo.height, bitmapInfo.width * 4, System.Drawing.Imaging.PixelFormat.Format32bppRgb, bitmapInfo.colorData);

            byte[] data = new byte[3* bitmapInfo.width * bitmapInfo.height];
            Marshal.Copy(bitmapInfo.colorData, data, 0, 3*bitmapInfo.width * bitmapInfo.height);

            Bitmap result = new Bitmap(bitmapInfo.width, bitmapInfo.height);
            for (int x = 0; x < result.Width; x++)
            {
                for (int y = 0; y < result.Height; y++)
                {
                    int idx = 3*(y*result.Width+x);
                    result.SetPixel(x,y, Color.FromArgb(data[idx], data[idx+1], data[idx+2]));
                }
            }

            result.Save(Path.Combine(dir, "segments", key));
        }

        private void SavePaletteToImage(Bitmap image, String filename, PaletteData data)
        {
            int colorSize = 100;
            int numColors = data.colors.Count();
            int gridWidth = 10;
            int padding = 0;

            int imageSize = 500;

            int imageWidth = imageSize;
            int imageHeight = imageSize;

            if (image.Width > image.Height)
                imageHeight = (int)Math.Round(imageSize / (double)image.Width * image.Height);
            else
                imageWidth = (int)Math.Round(imageSize / (double)image.Height * image.Width);

            int width = Math.Max(colorSize * Math.Min(gridWidth, numColors), imageSize) + 2 * padding;
            int height = imageHeight + 3 * padding + colorSize * (int)(Math.Ceiling(numColors / (double)gridWidth));

            Bitmap bitmap = new Bitmap(width, height);
            Graphics g = Graphics.FromImage(bitmap);


            //fill with black
            g.FillRectangle(new SolidBrush(Color.Black), 0, 0, bitmap.Width, bitmap.Height);


            //draw image
            g.DrawImage(image, padding, padding, imageWidth, imageHeight);

            //draw out the clusters
            for (int i = 0; i < numColors; i++)
            {
                int row = (int)Math.Floor(i / (double)gridWidth);
                int col = i - row * gridWidth;
                Pen pen = new Pen(data.colors[i]);
                g.FillRectangle(pen.Brush, col * colorSize + padding, imageHeight + 2 * padding + row * colorSize, colorSize - padding, colorSize - padding);

                double brightness = pen.Color.GetBrightness();
                Brush brush = new SolidBrush(Color.White);
                if (brightness > 0.5)
                    brush = new SolidBrush(Color.Black);

            }

            bitmap.Save(filename);

        }

        private void ExtractPalette(int pmethod)
        {
            Bitmap bmp = new Bitmap(currImageFile);//new Bitmap("../Data/bird.png");
            List<CIELAB> bmpData = Util.BitmapTo1DArray(bmp).Select(i => Util.RGBtoLAB(i)).ToList<CIELAB>();
            int k = Int32.Parse(KBox.Text);

            PaletteData data = new PaletteData();
            if (pmethod == 0)
            {
                if (!paletteCache.Exists("kmeans",k,currImageFile))
                {
                    List<Cluster> seeds = Clustering.InitializePictureSeeds(bmpData, Int32.Parse(KBox.Text));
                    Clustering.KMeansPicture(bmpData, seeds);

                    data.lab = seeds.Select(c => c.lab).ToList<CIELAB>();
                    data.colors = data.lab.Select(c => Util.LABtoRGB(c)).ToList<Color>();
                    paletteCache.SavePalette("kmeans",k,currImageFile,data);
                }
                else
                {
                    data = paletteCache.GetPalette("kmeans",k,currImageFile);
                }
            }
            else
            {
                if (!paletteCache.Exists("chidebug",k, currImageFile))
                {
                    FileInfo info = new FileInfo(currImageFile);
                    String dir = info.DirectoryName;
                    String key = info.Name;

                    //Save saliency and segmentation images if needed
                    SaveSaliencyMap(dir, key);
                    SaveSegmentation(dir, key);

                    PaletteExtractor extractor = new PaletteExtractor(dir, "../Weights", "../Weights/c3_data.json");
                    data = extractor.HillClimbPalette(key, "_Judd", true, k, CHITrials);
                    paletteCache.SavePalette("chidebug", k, currImageFile, data);
                }
                else
                {
                    data = paletteCache.GetPalette("chidebug", k, currImageFile);
                }

            }
            currPalette = data;
        }

        private void ExtractLayers(int pmethod, int lmethod)
        {
            //Load a bitmap and extract the layers
            Bitmap bmp = new Bitmap(currImageFile);

            List<CIELAB> bmpData = Util.BitmapTo1DArray(bmp).Select(i => Util.RGBtoLAB(i)).ToList<CIELAB>();

            ExtractPalette(pmethod);
            PaletteData data = currPalette;

            if (lmethod == 0)
            {

                double[] palette = new double[data.colors.Count * 3];
                for (int i = 0; i < data.colors.Count; i++)
                {
                    Color color = data.colors[i];
                    palette[3 * i] = color.R / 255.0;
                    palette[3 * i + 1] = color.G / 255.0;
                    palette[3 * i + 2] = color.B / 255.0;
                }
                IntPtr palettePtr = Marshal.AllocHGlobal(Marshal.SizeOf(palette[0]) * palette.Length);

                //for now, pass in RGB format
                byte[] rgbData = new byte[bmp.Width * bmp.Height * 3];
                for (int idx = 0; idx < bmpData.Count; idx++)
                {
                    Color color = Util.LABtoRGB(bmpData[idx]);
                    rgbData[3 * idx] = (byte)color.R;
                    rgbData[3 * idx + 1] = (byte)color.G;
                    rgbData[3 * idx + 2] = (byte)color.B;
                }

                BCBitmapInfo bmpInfo = new BCBitmapInfo();
                bmpInfo.colorData = Marshal.AllocHGlobal(Marshal.SizeOf(rgbData[0]) * rgbData.Length);
                bmpInfo.width = bmp.Width;
                bmpInfo.height = bmp.Height;

                try
                {
                    Marshal.Copy(rgbData, 0, bmpInfo.colorData, rgbData.Length);
                    Marshal.Copy(palette, 0, palettePtr, palette.Length);
                }
                finally
                {
                }

                IntPtr layersUnmanaged = BCExtractLayers(baseCodeDLLContext, bmpInfo, palettePtr, data.colors.Count);
                Marshal.FreeHGlobal(bmpInfo.colorData);
                Marshal.FreeHGlobal(palettePtr);

                ProcessLayers(layersUnmanaged);
            }
            else
            {
                //grad descent, convex constraint
                layers = new Layers();
                layers.colors = data.colors.Select(c=>new DenseVector(new double[]{c.R, c.G, c.B})).ToList<DenseVector>();
                layers.layers = LayerExtract.SolveLayersGradDescent(bmp, layers.colors);
                layers.width = bmp.Width;
                layers.height = bmp.Height;

            }

            CreatePreviews();
        }

        private void ProcessLayers(IntPtr layersUnmanaged)
        {

            if (layersUnmanaged == (IntPtr)0) return;

            layers = new Layers();

            BCLayers layerSet = (BCLayers)Marshal.PtrToStructure(layersUnmanaged, typeof(BCLayers));
            IntPtr layerInfoUnmanaged = layerSet.layers;

            int numLayers = layerSet.numLayers;
            for (int i = 0; i < numLayers; i++)
            {
                IntPtr layerPtr = IntPtr.Add(layerInfoUnmanaged, Marshal.SizeOf(typeof(BCLayerInfo)) * i);
                BCLayerInfo layerInfo = (BCLayerInfo)Marshal.PtrToStructure(layerPtr, typeof(BCLayerInfo));
                int width = layerInfo.width;
                int height = layerInfo.height;

                layers.layers.Add(new double[width, height]);
                double[] data = new double[width * height];
                Marshal.Copy(layerInfo.weights, data, 0, width * height);

                for (int x = 0; x < width; x++)
                    for (int y = 0; y < height; y++)
                        layers.layers[i][x, y] = data[y * width + x];

                layers.width = width;
                layers.height = height;
                DenseVector color = new DenseVector(new double[] { layerInfo.d0*255, layerInfo.d1*255, layerInfo.d2*255 });
                layers.colors.Add(color);
            }

        }

        private void CreatePreviews()
        {
            //now visualize the first layer
            for (int l = 0; l < layers.layers.Count; l++)
            {
                Bitmap result = new Bitmap(layers.width, layers.height);
                for (int x = 0; x < result.Width; x++)
                    for (int y = 0; y < result.Height; y++)
                    {
                        double weight = layers.layers[l][x, y];

                        if (weight < 0)
                        {
                            result.SetPixel(x, y, Color.FromArgb((int)Clamp(Math.Abs(weight) * 255), 0, 0));
                        }
                        else if (weight > 1.01)
                        {
                            result.SetPixel(x, y, Color.FromArgb(0, 255, 0));
                        }
                        else
                        {
                            int d = (int)Math.Min(Math.Max(Math.Round(weight * 255), 0), 255);
                            result.SetPixel(x, y, Color.FromArgb(d, d, d));
                        }


                    }

                layers.previews.Add(result);
                result.Save("test" + l + ".png");
            }
        }


        private void UpdatePaletteDisplay(bool enableEvents=true)
        {
            palettePanel.Controls.Clear();
            palette.Clear();

            ColorSpace cspace = layers.space;
            int padding = 10;

            for (int l = 0; l < currPalette.colors.Count; l++)
            {
                Color rgb = currPalette.colors[l];

                Button element = new Button();
                element.BackColor = rgb;
                element.Width = 150;
                element.Height = 20;
                element.Top = element.Height * l + padding;
                element.FlatStyle = FlatStyle.Flat;

                if (enableEvents)
                {
                    element.MouseHover += delegate(Object sender, EventArgs e)
                    {
                        Button btn = (Button)sender;
                        int index = palette.IndexOf(btn);

                        //preview the layer
                        layerBox.Image = layers.previews[index];

                    };

                    element.Click += delegate(Object sender, EventArgs e)
                    {
                        //Open the color picker and recolor 
                        Button btn = ((Button)sender);

                        colorPicker.Color = btn.BackColor;
                        colorPicker.ShowDialog();

                        if (colorPicker.Color != btn.BackColor)
                        {
                            btn.BackColor = colorPicker.Color;

                            //recolor
                            pictureBox.Image = Recolor(palette.Select(c => new DenseVector(new double[] { c.BackColor.R, c.BackColor.G, c.BackColor.B })).ToList<DenseVector>(), ColorSpace.RGB);


                        }


                    };
                }

                palette.Add(element);
                palettePanel.Controls.Add(element);
            }

        }



        private Bitmap GetBitmap(String bitmapName)
        {
            IntPtr bitmapInfoUnmanaged = BCQueryBitmapByName(baseCodeDLLContext, bitmapName);
            if (bitmapInfoUnmanaged == (IntPtr)0) return null;

            BCBitmapInfo bitmapInfo = (BCBitmapInfo)Marshal.PtrToStructure(bitmapInfoUnmanaged, typeof(BCBitmapInfo));

            return new Bitmap(bitmapInfo.width, bitmapInfo.height, bitmapInfo.width * 4, System.Drawing.Imaging.PixelFormat.Format32bppRgb, bitmapInfo.colorData);
        }

        private String GetString(String stringName)
        {
            IntPtr stringPtr = BCQueryStringByName(baseCodeDLLContext, stringName);
            if (stringPtr == (IntPtr)0)
            {
                return null;
            }
            else
            {
                return Marshal.PtrToStringAnsi(stringPtr);
            }
        }

        private void UpdateImages()
        {
            
            pictureBox.Image = (Image)GetBitmap("original");

            /*String color = GetString("layerColor_" + layerString);
            pictureBoxColor.BackColor = Color.FromArgb(Convert.ToInt32(color.Split(' ')[0]),
                                                       Convert.ToInt32(color.Split(' ')[1]),
                                                       Convert.ToInt32(color.Split(' ')[2]));*/
        }

        private void MainWindow_Load(object sender, EventArgs e)
        {

        }


        private Bitmap Recolor(List<DenseVector> newColors, ColorSpace cspace)
        {
            int numLayers = layers.layers.Count;
            int width = layers.width;
            int height = layers.height;

            Bitmap result = new Bitmap(width, height);

            List<DenseVector> convertedColors = newColors;
            if (cspace != layers.space)
            {
                if (cspace == ColorSpace.RGB)
                {
                    //convert to LAB
                    convertedColors = newColors.Select(v =>
                    {
                        var color = Util.RGBtoLAB(Color.FromArgb((int)v[0], (int)v[1], (int)v[2]));
                        return new DenseVector(new double[] { color.L, color.A, color.B });
                    }).ToList<DenseVector>();

                }
                else
                {
                    //convert to RGB
                    convertedColors = newColors.Select(v =>
                    {
                        var color = Util.LABtoRGB(new CIELAB(v[0], v[1], v[2]));
                        return new DenseVector(new double[] { color.R, color.G, color.B });
                    }).ToList<DenseVector>();
                }
            }

            for (int x = 0; x < width; x++)
            {
                for (int y = 0; y < height; y++)
                {
                    DenseVector color = DenseVector.Create(3, i => 0);
                    for (int l=0; l<numLayers; l++)
                    {
                        color += layers.layers[l][x, y] * convertedColors[l];
                    }
                 
                    //convert to RGB if necessary
                    if (cspace == ColorSpace.LAB)
                    {
                        Color rgb = Util.LABtoRGB(new CIELAB(color[0], color[1], color[2]));
                        result.SetPixel(x,y, rgb);
                    } else
                    {
                        result.SetPixel(x,y, Color.FromArgb((int)Clamp(color[0]), (int)Clamp(color[1]), (int)Clamp(color[2])));
                    }

                }
            }

            return result;
        }

        private double Clamp(double channel)
        {
            return Math.Max(0, Math.Min(channel, 255));
        }

        private void ClearPalette()
        {
            palette.Clear();
            palettePanel.Controls.Clear();
            layerBox.Image = new Bitmap(100, 100);
            pictureBoxOriginal.Image = new Bitmap(100, 100);
        }


        private void openButton_Click(object sender, EventArgs e)
        {
            ClearPalette();
            OpenFileDialog dialog = new OpenFileDialog();
            dialog.Filter = "Image Files (*.png)|*.png";
            dialog.ShowDialog();

            if (dialog.FileName != "")
            {
                String filepath = dialog.FileName;
                Image image = Image.FromFile(filepath);
                pictureBox.Image = image;
                currImageFile = filepath;
            }
        }

        private void extractLayersButton_Click(object sender, EventArgs e)
        {
            if (bw.IsBusy)
                return;

            this.Cursor = Cursors.WaitCursor;
            int method = paletteMethodBox.SelectedIndex;
            int layerMethod = layerMethodBox.SelectedIndex;

            bw = new BackgroundWorker();
            bw.DoWork += delegate { 
                ExtractLayers(method, layerMethod);
            };
            bw.RunWorkerCompleted += delegate
            {
                pictureBox.Image = Recolor(layers.colors, layers.space);
                UpdatePaletteDisplay(true);
                pictureBoxOriginal.Image = new Bitmap(currImageFile);
                this.Cursor = Cursors.Default;
            };
            bw.RunWorkerAsync();
        }

        private void saveButton_Click(object sender, EventArgs e)
        {
            SaveFileDialog dialog = new SaveFileDialog();
            dialog.Filter = "Image Files (*.png)|*.png";
            dialog.ShowDialog();

            if (dialog.FileName != "")
            {
                pictureBoxOriginal.Image.Save(dialog.FileName);
            }
            
        }

        private void resetImageButton_Click(object sender, EventArgs e)
        {
            //reset the palette and image
            for (int i = 0; i < palette.Count; i++)
            {
                DenseVector color = layers.colors[i];
                Color rgb = Color.White;
                if (layers.space == ColorSpace.LAB)
                    rgb = Util.LABtoRGB(new CIELAB(color[0], color[1], color[2]));
                else
                    rgb = Color.FromArgb((int)color[0], (int)color[1], (int)color[2]);

                palette[i].BackColor = rgb;
      
            }
            pictureBox.Image = Recolor(palette.Select(c => new DenseVector(new double[] { c.BackColor.R, c.BackColor.G, c.BackColor.B })).ToList<DenseVector>(), ColorSpace.RGB);

        }

        private void extractPaletteButton_Click(object sender, EventArgs e)
        {
            if (bw.IsBusy)
                return;

            this.Cursor = Cursors.WaitCursor;
            int method = paletteMethodBox.SelectedIndex;
            ClearPalette();

            bw = new BackgroundWorker();
            bw.DoWork += delegate
            {
                ExtractPalette(method);
            };
            bw.RunWorkerCompleted += delegate
            {
                UpdatePaletteDisplay(false);
                this.Cursor = Cursors.Default;
            };
            bw.RunWorkerAsync();
        }
    }
}
