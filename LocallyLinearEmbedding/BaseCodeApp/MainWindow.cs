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

        IntPtr baseCodeDLLContext = (IntPtr)0;

        public MainWindow()
        {
            InitializeComponent();

            if (baseCodeDLLContext == (IntPtr)0)
            {
                baseCodeDLLContext = BCInit();
            }

            layerScroll.Minimum = 1;
            layerScroll.Maximum = BCQueryIntegerByName(baseCodeDLLContext, "layerCount");
            layerScroll.Value = 1;
            UpdateImages();
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
            String layerString = (layerScroll.Value - 1).ToString();
            
            pictureBoxOriginal.Image = (Image)GetBitmap("original");

            /*String color = GetString("layerColor_" + layerString);
            pictureBoxColor.BackColor = Color.FromArgb(Convert.ToInt32(color.Split(' ')[0]),
                                                       Convert.ToInt32(color.Split(' ')[1]),
                                                       Convert.ToInt32(color.Split(' ')[2]));*/
        }

        private void MainWindow_Load(object sender, EventArgs e)
        {

        }

        private void buttonNewColor_Click(object sender, EventArgs e)
        {
            colorPicker.AnyColor = true;
            colorPicker.FullOpen = true;
            colorPicker.ShowHelp = false;
            colorPicker.Color = pictureBoxColor.BackColor;
            DialogResult result = colorPicker.ShowDialog();
            if (result == DialogResult.OK)
            {
                pictureBoxColor.BackColor = colorPicker.Color;
                BCProcessCommand(baseCodeDLLContext, "setLayerColor " + (layerScroll.Value - 1).ToString() + " " + pictureBoxColor.BackColor.R + " " + pictureBoxColor.BackColor.G + " " + pictureBoxColor.BackColor.B);
                UpdateImages();
            }
        }

        private void layerScroll_Scroll(object sender, ScrollEventArgs e)
        {
            labelLayer.Text = "Layer " + layerScroll.Value.ToString() + "/" + layerScroll.Maximum.ToString();
            UpdateImages();
        }

        private void buttonResetColor_Click(object sender, EventArgs e)
        {
            BCProcessCommand(baseCodeDLLContext, "resetLayerColor " + (layerScroll.Value - 1).ToString());
            UpdateImages();
        }
    }
}
