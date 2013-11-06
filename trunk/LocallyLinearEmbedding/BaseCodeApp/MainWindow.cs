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
using System.Windows.Input;

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
        private static extern IntPtr BCExtractLayers(IntPtr context, BCBitmapInfo bitmap, IntPtr palette, [In, MarshalAs(UnmanagedType.I4)]int paletteSize, [In, MarshalAs(UnmanagedType.LPStr)] String layerConstraints, [In,MarshalAs(UnmanagedType.Bool)] bool autoCorrect);
        [DllImport(BaseCodeDLL)]
        private static extern IntPtr BCSegmentImage(IntPtr context, BCBitmapInfo bitmap);
        [DllImport(BaseCodeDLL)]
        private static extern IntPtr BCSynthesizeLayers(IntPtr context);
        [DllImport(BaseCodeDLL)]
        private static extern IntPtr BCOutputMesh(IntPtr context, BCBitmapInfo bitmap, IntPtr palette, [In, MarshalAs(UnmanagedType.I4)]int paletteSize, [In, MarshalAs(UnmanagedType.LPStr)] String filename);

        IntPtr baseCodeDLLContext = (IntPtr)0;

        Image activeImage;
        Bitmap pictureBoxBitmap;
        Graphics g;

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

        public class LayerConstraints
        {
            public LayerConstraints(int layerCount)
            {
                constraints = new List<List<Point>>();
                foreach (int i in Enumerable.Range(1, layerCount)) constraints.Add(new List<Point>());
            }
            public List<List<Point>> constraints;

            public bool Valid()
            {
                return constraints.All(x => x.Count > 0);
            }
            public override string ToString()
            {
                if (!Valid()) return "";
                StringBuilder sb = new StringBuilder();
                foreach (var v in constraints)
                {
                    foreach (var p in v) sb.Append(p.X + "," + p.Y + ";");
                    sb.Remove(sb.Length - 1, 1);
                    sb.Append("|");
                }
                sb.Remove(sb.Length - 1, 1);
                return sb.ToString();
            }
        }
        LayerConstraints constraints;
        int activePaletteIndex;
        

        public MainWindow()
        {
            InitializeComponent();

            if (baseCodeDLLContext == (IntPtr)0)
            {
                baseCodeDLLContext = BCInit();
            }

            String[] commandLine = Environment.GetCommandLineArgs();
            if (commandLine.Length >= 3 && commandLine[1] == "--SynthesizeTexture")
            {
                BCProcessCommand(baseCodeDLLContext, "SynthesizeTexture " + commandLine[2]);
                throw new EarlyAbortException();
            }
            if (commandLine.Length >= 3 && commandLine[1] == "--SynthesizeTextureByLayers")
            {
                BCProcessCommand(baseCodeDLLContext, "SynthesizeTextureByLayers " + commandLine[2]);
                throw new EarlyAbortException();
            }

            //UpdateImages();
            paletteMethodBox.SelectedIndex = 1;
            layerMethodBox.SelectedIndex = 0;
            colorSpaceBox.SelectedIndex = 0;

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
            else if (pmethod == 1)
            {
                if (!paletteCache.Exists("chidebug", k, currImageFile))
                {
                    FileInfo info = new FileInfo(currImageFile);
                    String dir = info.DirectoryName;
                    String key = info.Name;

                    //Save saliency and segmentation images if needed
                    SaveSaliencyMap(dir, key);
                    SaveSegmentation(dir, key);
                    Bitmap resized = Util.GetImage(Path.Combine(dir, key), true);
                    resized.Save(key);

                    PaletteExtractor extractor = new PaletteExtractor(dir, "../Weights", "../Weights/c3_data.json");
                    data = extractor.HillClimbPalette(key, "_Judd", true, k, CHITrials);
                    paletteCache.SavePalette("chidebug", k, currImageFile, data);
                }
                else
                {
                    data = paletteCache.GetPalette("chidebug", k, currImageFile);
                }

            }
            else if (pmethod == 2)
            {
                data = ExtractConvexMergedPalette(k);
            }
            else if (pmethod == 3)
            {
                //encourage convex hull
   
            }
            else
            {
                //turk
                if (paletteCache.Exists("turk", 5, currImageFile))
                {
                    data = paletteCache.GetPalette("turk", 5, currImageFile);
                }

            }
            currPalette = data;
        }

        private void ExtractLayers(int pmethod, int lmethod, ColorSpace space)
        {
            //Load a bitmap and extract the layers
            Bitmap bmp = new Bitmap(currImageFile);

            List<CIELAB> bmpData = Util.BitmapTo1DArray(bmp).Select(i => Util.RGBtoLAB(i)).ToList<CIELAB>();

            //ExtractPalette(pmethod);
            //PaletteData data = currPalette;
            PaletteData data;
            if (currPalette.colors.Count() == 0)
            {
                ExtractPalette(pmethod);
                data = currPalette;
            }
            else
            {
                currPalette = new PaletteData();
                currPalette.colors = this.palette.Select(c => c.BackColor).ToList<Color>();
                currPalette.lab = this.palette.Select(c => Util.RGBtoLAB(c.BackColor)).ToList<CIELAB>();
                data = currPalette;
            }

            string constraintString = "";
            bool autoCorrect = autoBox.Checked;
            if (constraints != null) constraintString = constraints.ToString();

            if (lmethod == 0)
            {
                if (space == ColorSpace.RGB)
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

                    IntPtr layersUnmanaged = BCExtractLayers(baseCodeDLLContext, bmpInfo, palettePtr, data.colors.Count, constraintString, autoCorrect);
                    Marshal.FreeHGlobal(bmpInfo.colorData);
                    Marshal.FreeHGlobal(palettePtr);

                    layers = ProcessLayers(layersUnmanaged, space);
                }
                else
                {
                    double[] palette = new double[data.colors.Count * 3];
                    for (int i = 0; i < data.lab.Count; i++)
                    {
                        CIELAB color = data.lab[i];
                        palette[3 * i] = color.L / 255.0;
                        palette[3 * i + 1] = (color.A + 128) / 255.0;
                        palette[3 * i + 2] = (color.B + 128) / 255.0;
                    }
                    IntPtr palettePtr = Marshal.AllocHGlobal(Marshal.SizeOf(palette[0]) * palette.Length);

                    //for now, pass in RGB format
                    byte[] labData = new byte[bmp.Width * bmp.Height * 3];
                    for (int idx = 0; idx < bmpData.Count; idx++)
                    {
                        CIELAB color = bmpData[idx];
                        labData[3 * idx] = (byte)(color.L);
                        labData[3 * idx + 1] = (byte)(color.A + 128);
                        labData[3 * idx + 2] = (byte)(color.B + 128);
                    }

                    BCBitmapInfo bmpInfo = new BCBitmapInfo();
                    bmpInfo.colorData = Marshal.AllocHGlobal(Marshal.SizeOf(labData[0]) * labData.Length);
                    bmpInfo.width = bmp.Width;
                    bmpInfo.height = bmp.Height;

                    try
                    {
                        Marshal.Copy(labData, 0, bmpInfo.colorData, labData.Length);
                        Marshal.Copy(palette, 0, palettePtr, palette.Length);
                    }
                    finally
                    {
                    }

                    IntPtr layersUnmanaged = BCExtractLayers(baseCodeDLLContext, bmpInfo, palettePtr, data.colors.Count, constraintString, autoCorrect);
                    Marshal.FreeHGlobal(bmpInfo.colorData);
                    Marshal.FreeHGlobal(palettePtr);

                    layers = ProcessLayers(layersUnmanaged, space);
                }
            }
            else
            {
                //grad descent, convex constraint
                layers = new Layers();
                layers.space = space;

                if (layers.space == ColorSpace.RGB)
                {

                    DenseVector[,] bmpColors = Util.Map(Util.BitmapToArray(bmp), c => new DenseVector(new double[]{c.R, c.G, c.B}));
                    layers.colors = data.colors.Select(c => new DenseVector(new double[] { c.R, c.G, c.B })).ToList<DenseVector>();
                    layers.layers = LayerExtract.SolveLayersGradDescent(bmpColors, layers.colors);
                }
                else
                {
                    DenseVector[,] bmpColors = Util.Map(Util.BitmapToArray(bmp), c => { CIELAB l = Util.RGBtoLAB(c); return new DenseVector(new double[] { l.L, l.A, l.B});});
                    layers.colors = data.lab.Select(c => new DenseVector(new double[] { c.L, c.A, c.B })).ToList<DenseVector>();
                    layers.layers = LayerExtract.SolveLayersGradDescent(bmpColors, layers.colors);
                }
                
                layers.width = bmp.Width;
                layers.height = bmp.Height;

            }
            CreatePreviews();
        }

        private Layers ProcessLayers(IntPtr layersUnmanaged, ColorSpace space)
        {
            //process the layers and update the palette
            if (layersUnmanaged == (IntPtr)0) return new Layers();

            Layers layers = new Layers();
            layers.space = space;

            currPalette.colors.Clear();
            currPalette.lab.Clear();

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

                DenseVector color = new DenseVector(3);
                if (layers.space == ColorSpace.RGB)
                {
                    color = new DenseVector(new double[] { layerInfo.d0 * 255, layerInfo.d1 * 255, layerInfo.d2 * 255 });
                    Color rgb = Color.FromArgb((int)color[0], (int)color[1], (int)color[2]);
                    currPalette.colors.Add(rgb);
                    currPalette.lab.Add(Util.RGBtoLAB(rgb));
                }
                else
                {
                    color = new DenseVector(new double[] { layerInfo.d0 * 255, layerInfo.d1 * 255 - 128, layerInfo.d2 * 255 - 128 });
                    CIELAB lab = new CIELAB(color[0], color[1], color[2]);
                    currPalette.lab.Add(lab);
                    currPalette.colors.Add(Util.LABtoRGB(lab));
                }
                layers.colors.Add(color);
            }


           
            return layers;

        }

        private void CreatePreviews()
        {


            //print the fraction of negative pixels also within the (RGB) convex hull
            List<Point> points = new List<Point>();
            double negPixels = 0;
            double negCount = 0;
            Bitmap image = new Bitmap(currImageFile);
            var hull = Util.GetConvexHull(currPalette.colors.Select<Color, DenseVector>(c => new DenseVector(new double[] { c.R, c.G, c.B })).ToList<DenseVector>());
            for (int i = 0; i < layers.width; i++)
            {
                for (int j = 0; j < layers.height; j++)
                {
                    bool neg = false;
                    for (int l = 0; l < layers.layers.Count(); l++)
                    {
                        if (layers.layers[l][i, j] < 0)
                        {
                            neg = true;
                            break;
                        }
                    }
                    if (neg)
                    {
                        negPixels++;
                        Color rgb = image.GetPixel(i, j);
                        if (Util.InHull(new DenseVector(new double[] { rgb.R, rgb.G, rgb.B }), hull))
                        {
                            negCount++;
                            points.Add(new Point(i, j));
                        }

                    }
                }
            }
            Console.WriteLine("Negative pixels within convex hull: " + negCount / negPixels);
               

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

                /*foreach (Point p in points)
                {
                    if (layers.layers[l][p.X, p.Y] < 0)
                    {
                        double weight = layers.layers[l][p.X, p.Y];
                        result.SetPixel(p.X, p.Y, Color.FromArgb(0,0,(int)Clamp(Math.Abs(weight) * 255)));
                    }
                }*/


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
                        activePaletteIndex = index;

                        if (layers.colors.Count == 0)
                        {
                            UpdateConstraintVisualization();
                        }
                        else
                        {
                            //preview the layer
                            layerBox.Image = layers.previews[index];
                        }

                    };

                    element.MouseUp += delegate(Object sender, System.Windows.Forms.MouseEventArgs e)
                    {
                        //Open the color picker and recolor
                        Button btn = ((Button)sender);
                        int index = palette.IndexOf(btn);
                        if (e.Button == MouseButtons.Left)
                        {
                            colorPicker.Color = btn.BackColor;
                            colorPicker.ShowDialog();

                            if (colorPicker.Color != btn.BackColor)
                            {
                                btn.BackColor = colorPicker.Color;

                                if (layers.colors.Count > 0)
                                {
                                    //recolor
                                    pictureBox.Image = Recolor(layers, palette.Select(c => new DenseVector(new double[] { c.BackColor.R, c.BackColor.G, c.BackColor.B })).ToList<DenseVector>(), ColorSpace.RGB);
                                }
                            }
                        }
                        else if (e.Button == MouseButtons.Right)
                        {
                            constraints.constraints[index].Clear();
                            UpdateConstraintVisualization();
                        }
                    };
                }

                palette.Add(element);
                palettePanel.Controls.Add(element);
            }

            ColorSpace space = (colorSpaceBox.SelectedIndex == 0) ? ColorSpace.RGB : ColorSpace.LAB;

            //count convex hull
            if (currPalette.colors.Count > 3)
            {
                statusBox.Text = "Convex hull error: " + WithinConvexHull(space);

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


        /*private Dictionary<String, List<PaletteData>> LoadFilePalettes(String file)
        {
            //load art palettes
            var lines = File.ReadLines(file);

            Dictionary<String, List<PaletteData>> plist = new Dictionary<String, List<PaletteData>>();

            int count = 0;
            List<String> headers = new List<String>();

            foreach (String line in lines)
            {
                if (count == 0)
                {
                    count++;
                    headers = line.Replace("\"", "").Split('\t').ToList<String>();
                    continue;
                }

                String[] fields = line.Replace("\"", "").Split('\t');
                PaletteData data = new PaletteData();
                data.id = Int32.Parse(fields[headers.IndexOf("pid")]);
                data.workerNum = Int32.Parse(fields[headers.IndexOf("id")]);
                String key = fields[headers.IndexOf("image")];
                String[] colors = fields[headers.IndexOf("colors")].Split(new string[] { " " }, StringSplitOptions.RemoveEmptyEntries);
                if (headers.IndexOf("log") >= 0)
                    data.log = fields[headers.IndexOf("log")];

                foreach (String s in colors)
                {
                    String[] comp = s.Split(',');
                    Color c = Color.FromArgb(Int32.Parse(comp[0]), Int32.Parse(comp[1]), Int32.Parse(comp[2]));
                    CIELAB l = Util.RGBtoLAB(c);
                    data.colors.Add(c);
                    data.lab.Add(l);
                }
                if (!plist.ContainsKey(key))
                    plist.Add(key, new List<PaletteData>());
                plist[key].Add(data);
            }
            return plist;
        }*/



        private Bitmap Recolor(Layers layers, List<DenseVector> newColors, ColorSpace cspace)
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
                    if (layers.space == ColorSpace.LAB)
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
            currPalette = new PaletteData();
            palette.Clear();
            palettePanel.Controls.Clear();
            layerBox.Image = new Bitmap(100, 100);
            pictureBoxOriginal.Image = new Bitmap(100, 100);
            layers = new Layers();
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
                activeImage = Image.FromFile(filepath);
                pictureBox.Image = activeImage;
                pictureBoxBitmap = new Bitmap(pictureBox.Image.Width, pictureBox.Image.Height);
                g = Graphics.FromImage(pictureBoxBitmap);
                currImageFile = filepath;

                String basename = new FileInfo(filepath).Name;
                if (paletteCache.Exists("turk",5,basename) && !paletteMethodBox.Items.Contains("TurkAverage"))
                {
                    paletteMethodBox.Items.Add("TurkAverage");
                } else if (paletteMethodBox.Items.Contains("TurkAverage"))
                    paletteMethodBox.Items.Remove("TurkAverage");

            }
        }

        private void textureSynthesisButton_Click(object sender, EventArgs e)
        {
            BCProcessCommand(baseCodeDLLContext, "SynthesizeTexture");
        }
        private void textureByLayerButton_Click(object sender, EventArgs e)
        {
            BCProcessCommand(baseCodeDLLContext, "SynthesizeTextureByLayers");
        }

        private void extractLayersButton_Click(object sender, EventArgs e)
        {
            if (bw.IsBusy)
                return;

            this.Cursor = System.Windows.Forms.Cursors.WaitCursor;
            int method = paletteMethodBox.SelectedIndex;
            int layerMethod = layerMethodBox.SelectedIndex;
            ColorSpace space = (colorSpaceBox.SelectedIndex == 0) ? ColorSpace.RGB : ColorSpace.LAB;

            bw = new BackgroundWorker();
            bw.DoWork += delegate { 
                ExtractLayers(method, layerMethod, space);
            };
            bw.RunWorkerCompleted += delegate
            {
                pictureBox.Image = Recolor(layers, layers.colors, layers.space);
                UpdatePaletteDisplay(true);
                pictureBoxOriginal.Image = new Bitmap(currImageFile);
                this.Cursor = System.Windows.Forms.Cursors.Default;
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
                pictureBox.Image.Save(dialog.FileName);
            }
            
        }

        private void resetImageButton_Click(object sender, EventArgs e)
        {
            //reset the palette and image
            for (int i = 0; i < currPalette.colors.Count; i++)
            {
                Color rgb = currPalette.colors[i];
                palette[i].BackColor = rgb;
      
            }
            if (layers.layers.Count > 0)
                pictureBox.Image = Recolor(layers, palette.Select(c => new DenseVector(new double[] { c.BackColor.R, c.BackColor.G, c.BackColor.B })).ToList<DenseVector>(), ColorSpace.RGB);
            else
                pictureBox.Image = new Bitmap(currImageFile);

        }

        private void extractPaletteButton_Click(object sender, EventArgs e)
        {
            if (bw.IsBusy)
                return;

            this.Cursor = System.Windows.Forms.Cursors.WaitCursor;
            int method = paletteMethodBox.SelectedIndex;
            ClearPalette();
            resetImageButton_Click(sender, e);

            bw = new BackgroundWorker();
            bw.DoWork += delegate
            {
                ExtractPalette(method);
            };
            bw.RunWorkerCompleted += delegate
            {
                constraints = new LayerConstraints(currPalette.colors.Count);
                UpdatePaletteDisplay(true);
                this.Cursor = System.Windows.Forms.Cursors.Default;
            };
            bw.RunWorkerAsync();
        }

        //check percent of image within palette convex hull
        private double WithinConvexHull(ColorSpace space)
        {
            //check color space
            Color[] imageColors = Util.BitmapTo1DArray(new Bitmap(currImageFile));
            int numPoints = imageColors.Length;
            double error = 0;

            if (space == ColorSpace.LAB)
            {
                DenseVector[] points = imageColors.Select(c => { CIELAB l = Util.RGBtoLAB(c); return new DenseVector(new double[] { l.L, l.A, l.B }); }).ToArray<DenseVector>();
                List<DenseVector> hullPoints = currPalette.lab.Select(l => new DenseVector(new double[]{l.L, l.A, l.B})).ToList<DenseVector>();
                var hull = Util.GetConvexHull(hullPoints);
                foreach (DenseVector point in points)
                    error += Util.HullError(point, hull);
                 Console.WriteLine("Palette convex hull size: "+ hull.Points.Count());

            }
            else
            {
                DenseVector[] points = imageColors.Select(c => new DenseVector(new double[]{c.R, c.G, c.B})).ToArray<DenseVector>();
                List<DenseVector> hullPoints = currPalette.colors.Select(c => new DenseVector(new double[]{c.R, c.G, c.B})).ToList<DenseVector>();
                var hull = Util.GetConvexHull(hullPoints);
                foreach (DenseVector point in points)
                    error += Util.HullError(point, hull);

                Console.WriteLine("Palette convex hull size: " + hull.Points.Count());
                       
            }
           
            return error / imageColors.Length ;
        }

        private PaletteData ExtractConvexMergedPalette(int k)
        {
            //and in the meantime, output intermediate results
            Color[] imageColors = Util.BitmapTo1DArray(Util.GetImage(currImageFile, true));

            List<DenseVector> points = new List<DenseVector> { };
            String space = "rgb";

            if (layers.space == ColorSpace.LAB)
            {
                points = imageColors.Select(c => { CIELAB l = Util.RGBtoLAB(c); return new DenseVector(new double[] { l.L, l.A, l.B }); }).ToList<DenseVector>();
                space = "lab";
            }
            else
                points = imageColors.Select(c => new DenseVector(new double[] { c.R, c.G, c.B })).ToList<DenseVector>();

            points = points.Distinct<DenseVector>().ToList<DenseVector>();

            if (paletteCache.Exists("convexhullmerged-"+space, k, currImageFile))
                return paletteCache.GetPalette("convexhullmerged-"+space, k, currImageFile);

            Console.WriteLine("Computing convex hull");

            var hull = Util.GetConvexHull(points);
            var hullPoints = hull.Points.Select(p => p.Position).ToList<double[]>();
            var numPoints = hullPoints.Count;
            Console.WriteLine("ConvexPalette: Convex Hull # points: " + numPoints);

            //convert the points to LAB space, to merge in LAB space
            List<CIELAB> labColors = new List<CIELAB>();
            if (layers.space == ColorSpace.LAB)
                labColors = hullPoints.Select(p => new CIELAB(p[0], p[1], p[2])).ToList<CIELAB>();
            else
                labColors = hullPoints.Select(p=>Util.RGBtoLAB(Color.FromArgb((int)p[0],(int)p[1],(int)p[2]))).ToList<CIELAB>();


            //Run agglomerative clustering
            var clusterer = new AgglomerativeClustering();
            clusterer.ClusterColors(labColors, k);

            PaletteData data = new PaletteData();
            data.lab = clusterer.GetClusterColors();
            data.colors = data.lab.Select(l => Util.LABtoRGB(l)).ToList<Color>();

            paletteCache.SavePalette("convexhullmerged-"+space, k, currImageFile, data);

            return data;
        }



        private PaletteData ExtractConvexPalette(int rawK)
        {
            int k = Math.Max(rawK, 4);

            //and in the meantime, output intermediate results
            Color[] imageColors = Util.BitmapTo1DArray(Util.GetImage(currImageFile, true));

            List<DenseVector> points = new List<DenseVector> { };
            String space = "rgb";

            if (layers.space == ColorSpace.LAB)
            {
                points = imageColors.Select(c => { CIELAB l = Util.RGBtoLAB(c); return new DenseVector(new double[] { l.L, l.A, l.B }); }).ToList<DenseVector>();
                space = "lab";
            } 
            else
                points = imageColors.Select(c => new DenseVector(new double[] { c.R, c.G, c.B })).ToList<DenseVector>();

            if (paletteCache.Exists("convexhulldebug-"+space, k, currImageFile))
                return paletteCache.GetPalette("convexhulldebug-"+space, k, currImageFile);

            Console.WriteLine("Computing convex hull");

            var hull = Util.GetConvexHull(points);
            var hullPoints = hull.Points.Select(p=>p.Position).ToList<double[]>();
            var numPoints = hullPoints.Count;

            k = Math.Min(numPoints, k);

            if (paletteCache.Exists("convexhulldebug-"+space, k, currImageFile))
                return paletteCache.GetPalette("convexhulldebug-"+space, k, currImageFile);

            PaletteData temp = new PaletteData();
            if (layers.space == ColorSpace.LAB)
            {
                temp.lab = hullPoints.Select(p => new CIELAB(p[0], p[1], p[2])).ToList<CIELAB>();
                temp.colors = temp.lab.Select(l => Util.LABtoRGB(l)).ToList<Color>();
            }
            else
            {
                temp.colors = hullPoints.Select(p => Color.FromArgb((int)p[0], (int)p[1], (int)p[2])).ToList<Color>();
                temp.lab = temp.colors.Select(c => Util.RGBtoLAB(c)).ToList<CIELAB>();
            }
            paletteCache.SavePalette("convexhulldebug-" + space, numPoints, currImageFile, temp);

            var curPoints = new List<double[]>(hullPoints);
            Console.WriteLine("ConvexPalette: Convex Hull # points: " + curPoints.Count);
            
            //for each point, figure out which is best to remove
            while (curPoints.Count() > k)
            {
                //find the best point to remove
                int bestIdx = 0;
                double bestScore = 0;

                for (int i = 0; i < curPoints.Count(); i++)
                {
                    var option = new List<double[]>(curPoints);
                    option.RemoveAt(i);
                    double score = Util.NumInHull(points, Util.GetConvexHull(option));
                    if (score > bestScore)
                    {
                        bestIdx = i;
                        bestScore = score;
                    }
                }
                curPoints.RemoveAt(bestIdx);

                //save
                temp = new PaletteData();
                if (layers.space == ColorSpace.LAB)
                {
                    temp.lab = curPoints.Select(p => new CIELAB(p[0], p[1], p[2])).ToList<CIELAB>();
                    temp.colors = temp.lab.Select(l => Util.LABtoRGB(l)).ToList<Color>();
                }
                else
                {
                    temp.colors = curPoints.Select(p => Color.FromArgb((int)p[0], (int)p[1], (int)p[2])).ToList<Color>();
                    temp.lab = temp.colors.Select(c => Util.RGBtoLAB(c)).ToList<CIELAB>();
                }
                paletteCache.SavePalette("convexhulldebug-" + space, curPoints.Count, currImageFile, temp);
                Console.WriteLine("ConvexPalette: Solved k= " + curPoints.Count);
            }

            return temp;
        }

        private void buttonSaveConstraints_Click(object sender, EventArgs e)
        {

        }

        private void pictureBox_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            try
            {
                bool shiftPressed = Keyboard.IsKeyDown(Key.LeftShift) || Keyboard.IsKeyDown(Key.RightShift);
                if (shiftPressed)
                {
                    palette[activePaletteIndex].BackColor = pictureBoxBitmap.GetPixel(e.X, e.Y);
                }
                else
                {
                    constraints.constraints[activePaletteIndex].Add(new Point(e.X, e.Y));
                }
            }
            catch { }
            UpdateConstraintVisualization();
        }

        private void pictureBox_Click(object sender, EventArgs e)
        {

        }

        private void UpdateConstraintVisualization()
        {
            if (g == null) return;
            //g.DrawImage(activeImage, new Rectangle(0, 0, pictureBoxBitmap.Width, pictureBoxBitmap.Height), new Rectangle(0, 0, activeImage.Width, activeImage.Height), GraphicsUnit.Pixel);
            g.DrawImage(activeImage, new Point(0, 0));
            Pen p = new Pen(Color.Black, 2.0f);
            Brush b = new SolidBrush(palette[activePaletteIndex].BackColor);
            foreach (Point v in constraints.constraints[activePaletteIndex])
            {
                Rectangle r = new Rectangle(new Point(v.X - 5, v.Y - 5), new Size(10, 10));
                g.DrawEllipse(p, r);
                g.FillEllipse(b, r);
            }
            pictureBox.Image = pictureBoxBitmap;
        }

        private void layerSynthesisButton_Click(object sender, EventArgs e)
        {
            //do a test for now
            IntPtr layersUnmanaged = BCSynthesizeLayers(baseCodeDLLContext);
            Layers synth = ProcessLayers(layersUnmanaged, ColorSpace.RGB);
            Bitmap result = Recolor(synth, synth.colors, ColorSpace.RGB);
            result.Save("synthesized.png");

            LayerSynthesisWindow window = new LayerSynthesisWindow();
            window.ShowDialog();
        }

        private void saveLayersButton_Click(object sender, EventArgs e)
        {
            Directory.CreateDirectory("../Layers");
            String basename = new FileInfo(currImageFile).Name;

            //save the layers as pngs
            int layerIndex = 0;
            foreach (Bitmap layer in layers.previews)
            {
                Bitmap result = new Bitmap(layer);
                Graphics g = Graphics.FromImage(result);
                g.FillRectangle(new SolidBrush(currPalette.colors[layerIndex]), 0, 0, 20, layer.Height);
    

                List<String> lines = new List<String>();
                lines.Add(layers.height + "\t" + layers.width);
                lines.Add(String.Join("\t", layers.colors[layerIndex].Select(d => d/255.0).ToArray<double>()));
                //save the layer text file
                for (int y = 0; y < layer.Height; y++)
                {
                    String[] fields = new String[layer.Width];

                    for (int x = 0; x < layer.Width; x++)
                        fields[x] = layers.layers[layerIndex][x, y].ToString();

                    lines.Add(String.Join("\t", fields));

                }
                File.AppendAllLines(Path.Combine("../Layers", Util.ConvertFileName(basename, "_" + layerIndex, ".txt")), lines.ToArray<String>());   
                            
                result.Save(Path.Combine("../Layers", Util.ConvertFileName(basename, "_"+layerIndex)));
                result.Dispose();
                layerIndex++;
            }

            Bitmap reconstructed = Recolor(layers, layers.colors, layers.space);
            reconstructed.Save(Path.Combine("../Layers", Util.ConvertFileName(basename, "_r")));

        }



        //
        // Color-suggestion-related methods
        //
        //Load PaletteData from file
        private Dictionary<String, PaletteData> LoadFilePalettes(String file)
        {
            FileInfo finfo = new FileInfo(file);

            Dictionary<String, PaletteData> plist = new Dictionary<String, PaletteData>();
            String[] lines = File.ReadAllLines(file);

            //extracted palettes file format
            if (finfo.Extension == ".tsv")
            {
                for (int i = 1; i < lines.Count(); i++)
                {
                    String line = lines[i];
                    String[] fields = line.Replace("\"", "").Split('\t');
                    PaletteData data = new PaletteData();
                    data.id = Int32.Parse(fields[0]);
                    data.workerNum = Int32.Parse(fields[1]);
                    String key = fields[2];
                    String[] colors = fields[3].Split(new string[] { " " }, StringSplitOptions.RemoveEmptyEntries);
                    foreach (String s in colors)
                    {
                        String[] comp = s.Split(',');
                        Color c = Color.FromArgb(Int32.Parse(comp[0]), Int32.Parse(comp[1]), Int32.Parse(comp[2]));
                        CIELAB l = Util.RGBtoLAB(c);
                        data.colors.Add(c);
                        data.lab.Add(l);
                    }
                    if (!plist.ContainsKey(key))
                        plist.Add(key, data);
                    else
                        throw new IOException("More than one palette per key");
                }
            }
            else //pattern template file format, populate the pattern id to template id field
            {
                for (int i = 0; i < lines.Count(); i++)
                {
                    String line = lines[i];
                    String[] fields = line.Replace("\"", "").Split(',');

                    PaletteData data = new PaletteData();
                    data.id = Int32.Parse(fields[1]);
                    data.workerName = fields[0];
                    String[] colors = fields[3].Split(new string[] { " " }, StringSplitOptions.RemoveEmptyEntries);
                    colors = colors.Distinct<String>().ToArray<String>();
                    String key = fields[1] + ".png";
                    foreach (String s in colors)
                    {
                        Color c = ColorTranslator.FromHtml("#" + s);
                        data.colors.Add(c);
                        data.lab.Add(Util.RGBtoLAB(c));

                    }

                    //ignore missing palette data
                    if (!plist.ContainsKey(key) && colors.Count() > 0)
                        plist.Add(key, data);
                    else if (plist.ContainsKey(key))
                        throw new IOException("More than one palette per key");

                    //String templateId = fields[4];
                    //if (!patternToTemplate.ContainsKey(data.id.ToString()))
                    //    patternToTemplate.Add(data.id.ToString(), templateId);


                }
            }
            return plist;
        }

        private void OutputMesh(String filename)
        {
            //save the mesh
            PaletteData data;
            int pmethod = paletteMethodBox.SelectedIndex;

            if (currPalette.colors.Count() == 0)
            {
                ExtractPalette(pmethod);
                data = currPalette;
            }
            else
            {
                currPalette = new PaletteData();
                currPalette.colors = this.palette.Select(c => c.BackColor).ToList<Color>();
                currPalette.lab = this.palette.Select(c => Util.RGBtoLAB(c.BackColor)).ToList<CIELAB>();
                data = currPalette;
            }

            //read and process the file
            Bitmap bmp = new Bitmap(currImageFile);
            List<Color> bmpData = Util.BitmapTo1DArray(bmp).ToList<Color>();

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
                Marshal.Copy(palette, 0, palettePtr, palette.Length);
            }
            finally
            {
            }

            BCOutputMesh(baseCodeDLLContext, bmpInfo, palettePtr, data.colors.Count, filename);
            Marshal.FreeHGlobal(bmpInfo.colorData);
            Marshal.FreeHGlobal(palettePtr);

        }

        private void outputMeshes_Click(object sender, EventArgs e)
        {
            //Hardcoded directory for now
            String directory = "../Training/CLdata";
            String paletteFile = "../Training/templates.csv";
            String outDir = "../Training/CLmeshes";

            Dictionary<String, PaletteData> palettes = LoadFilePalettes(paletteFile);

            String[] subdirs = Directory.GetDirectories(directory);
            foreach (String dir in subdirs)
            {
                String outSubDir = Path.Combine(outDir, new DirectoryInfo(dir).Name);
                Directory.CreateDirectory(outSubDir);
                String[] files = Directory.GetFiles(dir);
                foreach (String file in files)
                {
                    try
                    {
                        Console.WriteLine("Handling "+file);
                        String baseName = new FileInfo(file).Name;
                        String outFile = Path.Combine(outSubDir, Util.ConvertFileName(baseName, "", ".txt"));

                        //read and process the file
                        Bitmap bmp = new Bitmap(file);
                        List<Color> bmpData = Util.BitmapTo1DArray(bmp).ToList<Color>();

                        PaletteData data = palettes[baseName];

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
                            Color color = bmpData[idx];
                            rgbData[3 * idx] = (byte)color.R;
                            rgbData[3 * idx + 1] = (byte)color.G;
                            rgbData[3 * idx + 2] = (byte)color.B;
                        }
                        Console.WriteLine("Allocating");
                        BCBitmapInfo bmpInfo = new BCBitmapInfo();
                        bmpInfo.colorData = Marshal.AllocHGlobal(Marshal.SizeOf(rgbData[0]) * rgbData.Length);
                        bmpInfo.width = bmp.Width;
                        bmpInfo.height = bmp.Height;

                        try
                        {
                            Marshal.Copy(rgbData, 0, bmpInfo.colorData, rgbData.Length);
                            Marshal.Copy(palette, 0, palettePtr, palette.Length);
                        }
                        catch (Exception t)
                        {
                            Console.WriteLine(t.StackTrace);
                            Console.WriteLine(t.Message);
                        }
                        finally
                        {
                        }
                        Console.WriteLine("Outputting Mesh");
                        BCOutputMesh(baseCodeDLLContext, bmpInfo, palettePtr, data.colors.Count, outFile);
                        Console.WriteLine("Freeing");
                        Marshal.FreeHGlobal(bmpInfo.colorData);
                        Marshal.FreeHGlobal(palettePtr);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.StackTrace);
                        Console.WriteLine(ex.Message);
                    }

                }
            }

        }

        private void getRecoloringsButton_Click(object sender, EventArgs e)
        {
            //first extract layers
            int method = paletteMethodBox.SelectedIndex;
            int layerMethod = layerMethodBox.SelectedIndex;
            ColorSpace space = (colorSpaceBox.SelectedIndex == 0) ? ColorSpace.RGB : ColorSpace.LAB;
            ExtractLayers(method, layerMethod, space);
            UpdatePaletteDisplay();

            //first save the mesh
            OutputMesh("../Training/Models/mesh.txt");

            String exeDir = new DirectoryInfo("../Training/Models").FullName.Replace("\\","/");
            String jar = "colorinference.SuggestApp";
            Console.WriteLine("Starting " + jar);

            Console.WriteLine("Outputting suggestions");

            String dependencies = "-Djava.library.path=\"" +"."+";"+"dependencies" + "\"";
            String classPath = "-classpath \"" +"*.jar;" + "dependencies/*.jar" + ";" + "dependencies/*.dll"+ "\"";

            ProcessStartInfo info = new ProcessStartInfo();
            info.FileName = "\"C:/Program Files (x86)/scala/bin/scala.bat\"";
            info.WorkingDirectory = exeDir;

            info.Arguments = dependencies+" "+classPath+" "+jar+" suggest mesh.txt AlineDam 10 -lightness";
            info.UseShellExecute = false;
            info.RedirectStandardOutput = true;
            info.RedirectStandardError = true;
            info.CreateNoWindow = true;

            Console.WriteLine(info.FileName);
            Console.WriteLine(info.Arguments);
            File.WriteAllLines("test.txt", new String[] {info.FileName, info.Arguments});

            Process process = new Process();
            process.StartInfo = info;
            process.Start();

            String basename = new FileInfo(currImageFile).Name;
            int count = 0;
            while (!process.StandardError.EndOfStream)
            {
                String line = process.StandardError.ReadLine();
                Console.WriteLine(line);
            }

            bool start = false;
            while (!process.StandardOutput.EndOfStream)
            {
                string line = process.StandardOutput.ReadLine();
                if (line.StartsWith("Response"))
                {
                    start = true;
                    continue;
                }

                if (!start)
                    continue;

                Console.WriteLine("Processing suggestion " + count);

                // do something with line
                Console.WriteLine(line);

                String[] hexCodes = line.Split(',');
                List<DenseVector> colors = hexCodes.Select(h => ColorTranslator.FromHtml(h)).Select(c => new DenseVector(new double[]{c.R, c.G, c.B})).ToList<DenseVector>();

                // recolor the image
                Bitmap recoloring = Recolor(this.layers, colors, ColorSpace.RGB);

                recoloring.Save(Path.Combine("../ColorSuggestions/", Util.ConvertFileName(basename, "_"+count)));
                count++;
            }
            Bitmap orig = new Bitmap(currImageFile);
            orig.Save(Path.Combine("../ColorSuggestions/", basename));

            Console.WriteLine("Done");
        }

        private void trainModelButton_Click(object sender, EventArgs e)
        {
            String exeDir = new DirectoryInfo("../Training/Models").FullName.Replace("\\", "/");
            String jar = "colorinference.SuggestApp";
            Console.WriteLine("Starting " + jar);

            Console.WriteLine("Outputting suggestions");

            String dependencies = "-Djava.library.path=\"" + "." + ";" + "dependencies" + "\"";
            String classPath = "-classpath \"" + "*.jar;" + "dependencies/*.jar" + ";" + "dependencies/*.dll" + "\"";

            ProcessStartInfo info = new ProcessStartInfo();
            info.FileName = "\"C:/Program Files (x86)/scala/bin/scala.bat\"";
            info.WorkingDirectory = exeDir;

            info.Arguments = dependencies + " " + classPath + " " + jar + " train ../CLmeshes/AlineDam AlineDam";
            info.UseShellExecute = false;
            info.RedirectStandardOutput = true;
            info.RedirectStandardError = true;
            info.CreateNoWindow = true;

            Console.WriteLine(info.FileName);
            Console.WriteLine(info.Arguments);
            File.WriteAllLines("test.txt", new String[] { info.FileName, info.Arguments });

            Process process = new Process();
            process.StartInfo = info;
            process.Start();

        
            while (!process.StandardOutput.EndOfStream)
            {
                string line = process.StandardOutput.ReadLine();
                Console.WriteLine(line);
            }
            while (!process.StandardError.EndOfStream)
            {
                String line = process.StandardError.ReadLine();
                Console.WriteLine(line);
            }


            Console.WriteLine("Done");
        }

        private void buttonVideo_Click(object sender, EventArgs e)
        {
            BCProcessCommand(baseCodeDLLContext, "ExtractVideoLayers");
        }

    }

}
