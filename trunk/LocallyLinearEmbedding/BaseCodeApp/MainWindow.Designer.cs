namespace BaseCodeApp
{
    partial class MainWindow
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.pictureBoxOriginal = new System.Windows.Forms.PictureBox();
            this.colorPicker = new System.Windows.Forms.ColorDialog();
            this.layerScroll = new System.Windows.Forms.HScrollBar();
            this.pictureBoxLayer = new System.Windows.Forms.PictureBox();
            this.pictureBoxComposite = new System.Windows.Forms.PictureBox();
            this.buttonNewColor = new System.Windows.Forms.Button();
            this.pictureBoxColor = new System.Windows.Forms.PictureBox();
            this.labelLayer = new System.Windows.Forms.Label();
            this.buttonResetColor = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxOriginal)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxLayer)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxComposite)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxColor)).BeginInit();
            this.SuspendLayout();
            // 
            // pictureBoxOriginal
            // 
            this.pictureBoxOriginal.Location = new System.Drawing.Point(12, 83);
            this.pictureBoxOriginal.Name = "pictureBoxOriginal";
            this.pictureBoxOriginal.Size = new System.Drawing.Size(562, 804);
            this.pictureBoxOriginal.TabIndex = 0;
            this.pictureBoxOriginal.TabStop = false;
            // 
            // layerScroll
            // 
            this.layerScroll.LargeChange = 1;
            this.layerScroll.Location = new System.Drawing.Point(1147, 9);
            this.layerScroll.Name = "layerScroll";
            this.layerScroll.Size = new System.Drawing.Size(400, 26);
            this.layerScroll.TabIndex = 1;
            this.layerScroll.Scroll += new System.Windows.Forms.ScrollEventHandler(this.layerScroll_Scroll);
            // 
            // pictureBoxLayer
            // 
            this.pictureBoxLayer.Location = new System.Drawing.Point(1147, 83);
            this.pictureBoxLayer.Name = "pictureBoxLayer";
            this.pictureBoxLayer.Size = new System.Drawing.Size(562, 804);
            this.pictureBoxLayer.TabIndex = 0;
            this.pictureBoxLayer.TabStop = false;
            // 
            // pictureBoxComposite
            // 
            this.pictureBoxComposite.Location = new System.Drawing.Point(579, 83);
            this.pictureBoxComposite.Name = "pictureBoxComposite";
            this.pictureBoxComposite.Size = new System.Drawing.Size(562, 804);
            this.pictureBoxComposite.TabIndex = 0;
            this.pictureBoxComposite.TabStop = false;
            // 
            // buttonNewColor
            // 
            this.buttonNewColor.Location = new System.Drawing.Point(1147, 47);
            this.buttonNewColor.Name = "buttonNewColor";
            this.buttonNewColor.Size = new System.Drawing.Size(95, 30);
            this.buttonNewColor.TabIndex = 2;
            this.buttonNewColor.Text = "New Color";
            this.buttonNewColor.UseVisualStyleBackColor = true;
            this.buttonNewColor.Click += new System.EventHandler(this.buttonNewColor_Click);
            // 
            // pictureBoxColor
            // 
            this.pictureBoxColor.Location = new System.Drawing.Point(1350, 49);
            this.pictureBoxColor.Name = "pictureBoxColor";
            this.pictureBoxColor.Size = new System.Drawing.Size(358, 28);
            this.pictureBoxColor.TabIndex = 3;
            this.pictureBoxColor.TabStop = false;
            // 
            // labelLayer
            // 
            this.labelLayer.AutoSize = true;
            this.labelLayer.Location = new System.Drawing.Point(1560, 16);
            this.labelLayer.Name = "labelLayer";
            this.labelLayer.Size = new System.Drawing.Size(73, 19);
            this.labelLayer.TabIndex = 4;
            this.labelLayer.Text = "Layer 0/0";
            // 
            // buttonResetColor
            // 
            this.buttonResetColor.Location = new System.Drawing.Point(1248, 47);
            this.buttonResetColor.Name = "buttonResetColor";
            this.buttonResetColor.Size = new System.Drawing.Size(96, 30);
            this.buttonResetColor.TabIndex = 2;
            this.buttonResetColor.Text = "Reset Color";
            this.buttonResetColor.UseVisualStyleBackColor = true;
            this.buttonResetColor.Click += new System.EventHandler(this.buttonResetColor_Click);
            // 
            // MainWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 19F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1721, 902);
            this.Controls.Add(this.labelLayer);
            this.Controls.Add(this.pictureBoxColor);
            this.Controls.Add(this.buttonResetColor);
            this.Controls.Add(this.buttonNewColor);
            this.Controls.Add(this.layerScroll);
            this.Controls.Add(this.pictureBoxComposite);
            this.Controls.Add(this.pictureBoxLayer);
            this.Controls.Add(this.pictureBoxOriginal);
            this.Font = new System.Drawing.Font("Calibri", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "MainWindow";
            this.Text = "Main Window";
            this.Load += new System.EventHandler(this.MainWindow_Load);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxOriginal)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxLayer)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxComposite)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxColor)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox pictureBoxOriginal;
        private System.Windows.Forms.ColorDialog colorPicker;
        private System.Windows.Forms.HScrollBar layerScroll;
        private System.Windows.Forms.PictureBox pictureBoxLayer;
        private System.Windows.Forms.PictureBox pictureBoxComposite;
        private System.Windows.Forms.Button buttonNewColor;
        private System.Windows.Forms.PictureBox pictureBoxColor;
        private System.Windows.Forms.Label labelLayer;
        private System.Windows.Forms.Button buttonResetColor;
    }
}

