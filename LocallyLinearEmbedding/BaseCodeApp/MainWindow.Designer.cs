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
            this.pictureBox = new System.Windows.Forms.PictureBox();
            this.colorPicker = new System.Windows.Forms.ColorDialog();
            this.palettePanel = new System.Windows.Forms.Panel();
            this.openButton = new System.Windows.Forms.Button();
            this.extractLayersButton = new System.Windows.Forms.Button();
            this.layerBox = new System.Windows.Forms.PictureBox();
            this.label1 = new System.Windows.Forms.Label();
            this.KBox = new System.Windows.Forms.TextBox();
            this.saveButton = new System.Windows.Forms.Button();
            this.resetImageButton = new System.Windows.Forms.Button();
            this.paletteMethodBox = new System.Windows.Forms.ComboBox();
            this.paletteLabel = new System.Windows.Forms.Label();
            this.layerLabel = new System.Windows.Forms.Label();
            this.layerMethodBox = new System.Windows.Forms.ComboBox();
            this.extractPaletteButton = new System.Windows.Forms.Button();
            this.pictureBoxOriginal = new System.Windows.Forms.PictureBox();
            this.colorSpaceLabel = new System.Windows.Forms.Label();
            this.colorSpaceBox = new System.Windows.Forms.ComboBox();
            this.buttonSaveConstraints = new System.Windows.Forms.Button();
            this.buttonLoadConstraints = new System.Windows.Forms.Button();
            this.statusBox = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.layerSynthesisButton = new System.Windows.Forms.Button();
            this.saveLayersButton = new System.Windows.Forms.Button();
            this.textureSynthesisButton = new System.Windows.Forms.Button();
            this.autoBox = new System.Windows.Forms.CheckBox();
            this.textureByLayerButton = new System.Windows.Forms.Button();
            this.getRecoloringsButton = new System.Windows.Forms.Button();
            this.outputMeshes = new System.Windows.Forms.Button();
            this.trainModelButton = new System.Windows.Forms.Button();
            this.buttonVideo = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.layerBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxOriginal)).BeginInit();
            this.SuspendLayout();
            // 
            // pictureBox
            // 
            this.pictureBox.Location = new System.Drawing.Point(555, 12);
            this.pictureBox.Name = "pictureBox";
            this.pictureBox.Size = new System.Drawing.Size(576, 464);
            this.pictureBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.pictureBox.TabIndex = 0;
            this.pictureBox.TabStop = false;
            this.pictureBox.Click += new System.EventHandler(this.pictureBox_Click);
            this.pictureBox.MouseDown += new System.Windows.Forms.MouseEventHandler(this.pictureBox_MouseDown);
            // 
            // palettePanel
            // 
            this.palettePanel.AutoScroll = true;
            this.palettePanel.Location = new System.Drawing.Point(371, 12);
            this.palettePanel.Name = "palettePanel";
            this.palettePanel.Size = new System.Drawing.Size(169, 464);
            this.palettePanel.TabIndex = 1;
            // 
            // openButton
            // 
            this.openButton.Location = new System.Drawing.Point(29, 526);
            this.openButton.Name = "openButton";
            this.openButton.Size = new System.Drawing.Size(139, 35);
            this.openButton.TabIndex = 2;
            this.openButton.Text = "Open Image";
            this.openButton.UseVisualStyleBackColor = true;
            this.openButton.Click += new System.EventHandler(this.openButton_Click);
            // 
            // extractLayersButton
            // 
            this.extractLayersButton.Location = new System.Drawing.Point(173, 568);
            this.extractLayersButton.Name = "extractLayersButton";
            this.extractLayersButton.Size = new System.Drawing.Size(136, 35);
            this.extractLayersButton.TabIndex = 3;
            this.extractLayersButton.Text = "Extract Layers";
            this.extractLayersButton.UseVisualStyleBackColor = true;
            this.extractLayersButton.Click += new System.EventHandler(this.extractLayersButton_Click);
            // 
            // layerBox
            // 
            this.layerBox.Location = new System.Drawing.Point(12, 12);
            this.layerBox.Name = "layerBox";
            this.layerBox.Size = new System.Drawing.Size(349, 228);
            this.layerBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.layerBox.TabIndex = 4;
            this.layerBox.TabStop = false;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(25, 654);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(30, 19);
            this.label1.TabIndex = 5;
            this.label1.Text = "K =";
            // 
            // KBox
            // 
            this.KBox.Location = new System.Drawing.Point(61, 650);
            this.KBox.Name = "KBox";
            this.KBox.Size = new System.Drawing.Size(69, 27);
            this.KBox.TabIndex = 6;
            this.KBox.Text = "5";
            // 
            // saveButton
            // 
            this.saveButton.Location = new System.Drawing.Point(314, 526);
            this.saveButton.Name = "saveButton";
            this.saveButton.Size = new System.Drawing.Size(136, 34);
            this.saveButton.TabIndex = 3;
            this.saveButton.Text = "Save Image";
            this.saveButton.UseVisualStyleBackColor = true;
            this.saveButton.Click += new System.EventHandler(this.saveButton_Click);
            // 
            // resetImageButton
            // 
            this.resetImageButton.Location = new System.Drawing.Point(29, 567);
            this.resetImageButton.Name = "resetImageButton";
            this.resetImageButton.Size = new System.Drawing.Size(139, 37);
            this.resetImageButton.TabIndex = 7;
            this.resetImageButton.Text = "Reset Image";
            this.resetImageButton.UseVisualStyleBackColor = true;
            this.resetImageButton.Click += new System.EventHandler(this.resetImageButton_Click);
            // 
            // paletteMethodBox
            // 
            this.paletteMethodBox.FormattingEnabled = true;
            this.paletteMethodBox.Items.AddRange(new object[] {
            "K-Means",
            "CHI-Resize",
            "ConvexHull",
            "CHI-Patch"});
            this.paletteMethodBox.Location = new System.Drawing.Point(378, 649);
            this.paletteMethodBox.Name = "paletteMethodBox";
            this.paletteMethodBox.Size = new System.Drawing.Size(162, 27);
            this.paletteMethodBox.TabIndex = 8;
            // 
            // paletteLabel
            // 
            this.paletteLabel.AutoSize = true;
            this.paletteLabel.Location = new System.Drawing.Point(310, 652);
            this.paletteLabel.Name = "paletteLabel";
            this.paletteLabel.Size = new System.Drawing.Size(57, 19);
            this.paletteLabel.TabIndex = 9;
            this.paletteLabel.Text = "Palette";
            // 
            // layerLabel
            // 
            this.layerLabel.AutoSize = true;
            this.layerLabel.Location = new System.Drawing.Point(25, 700);
            this.layerLabel.Name = "layerLabel";
            this.layerLabel.Size = new System.Drawing.Size(105, 19);
            this.layerLabel.TabIndex = 10;
            this.layerLabel.Text = "Layer Method";
            // 
            // layerMethodBox
            // 
            this.layerMethodBox.FormattingEnabled = true;
            this.layerMethodBox.Items.AddRange(new object[] {
            "Manifold",
            "Pixel-ConvexConstraint"});
            this.layerMethodBox.Location = new System.Drawing.Point(134, 697);
            this.layerMethodBox.Name = "layerMethodBox";
            this.layerMethodBox.Size = new System.Drawing.Size(162, 27);
            this.layerMethodBox.TabIndex = 11;
            // 
            // extractPaletteButton
            // 
            this.extractPaletteButton.Location = new System.Drawing.Point(173, 526);
            this.extractPaletteButton.Name = "extractPaletteButton";
            this.extractPaletteButton.Size = new System.Drawing.Size(135, 35);
            this.extractPaletteButton.TabIndex = 12;
            this.extractPaletteButton.Text = "Extract Palette";
            this.extractPaletteButton.UseVisualStyleBackColor = true;
            this.extractPaletteButton.Click += new System.EventHandler(this.extractPaletteButton_Click);
            // 
            // pictureBoxOriginal
            // 
            this.pictureBoxOriginal.Location = new System.Drawing.Point(13, 258);
            this.pictureBoxOriginal.Name = "pictureBoxOriginal";
            this.pictureBoxOriginal.Size = new System.Drawing.Size(348, 218);
            this.pictureBoxOriginal.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pictureBoxOriginal.TabIndex = 13;
            this.pictureBoxOriginal.TabStop = false;
            // 
            // colorSpaceLabel
            // 
            this.colorSpaceLabel.AutoSize = true;
            this.colorSpaceLabel.Location = new System.Drawing.Point(310, 700);
            this.colorSpaceLabel.Name = "colorSpaceLabel";
            this.colorSpaceLabel.Size = new System.Drawing.Size(89, 19);
            this.colorSpaceLabel.TabIndex = 15;
            this.colorSpaceLabel.Text = "Color Space";
            // 
            // colorSpaceBox
            // 
            this.colorSpaceBox.FormattingEnabled = true;
            this.colorSpaceBox.Items.AddRange(new object[] {
            "RGB",
            "LAB"});
            this.colorSpaceBox.Location = new System.Drawing.Point(419, 697);
            this.colorSpaceBox.Name = "colorSpaceBox";
            this.colorSpaceBox.Size = new System.Drawing.Size(121, 27);
            this.colorSpaceBox.TabIndex = 16;
            // 
            // buttonSaveConstraints
            // 
            this.buttonSaveConstraints.Location = new System.Drawing.Point(29, 610);
            this.buttonSaveConstraints.Name = "buttonSaveConstraints";
            this.buttonSaveConstraints.Size = new System.Drawing.Size(139, 34);
            this.buttonSaveConstraints.TabIndex = 17;
            this.buttonSaveConstraints.Text = "Save Constraints";
            this.buttonSaveConstraints.UseVisualStyleBackColor = true;
            this.buttonSaveConstraints.Click += new System.EventHandler(this.buttonSaveConstraints_Click);
            // 
            // buttonLoadConstraints
            // 
            this.buttonLoadConstraints.Location = new System.Drawing.Point(173, 609);
            this.buttonLoadConstraints.Name = "buttonLoadConstraints";
            this.buttonLoadConstraints.Size = new System.Drawing.Size(136, 34);
            this.buttonLoadConstraints.TabIndex = 17;
            this.buttonLoadConstraints.Text = "Load Constraints";
            this.buttonLoadConstraints.UseVisualStyleBackColor = true;
            // 
            // statusBox
            // 
            this.statusBox.BackColor = System.Drawing.SystemColors.Control;
            this.statusBox.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.statusBox.Location = new System.Drawing.Point(-1, 730);
            this.statusBox.Name = "statusBox";
            this.statusBox.Size = new System.Drawing.Size(1230, 20);
            this.statusBox.TabIndex = 14;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(15, 500);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(438, 19);
            this.label2.TabIndex = 18;
            this.label2.Text = "Right-click: Reset constraints.  Shift-click on image: eyedropper";
            // 
            // layerSynthesisButton
            // 
            this.layerSynthesisButton.Location = new System.Drawing.Point(29, 756);
            this.layerSynthesisButton.Name = "layerSynthesisButton";
            this.layerSynthesisButton.Size = new System.Drawing.Size(159, 38);
            this.layerSynthesisButton.TabIndex = 19;
            this.layerSynthesisButton.Text = "Layer Synthesis";
            this.layerSynthesisButton.UseVisualStyleBackColor = true;
            this.layerSynthesisButton.Click += new System.EventHandler(this.layerSynthesisButton_Click);
            // 
            // saveLayersButton
            // 
            this.saveLayersButton.Location = new System.Drawing.Point(314, 567);
            this.saveLayersButton.Name = "saveLayersButton";
            this.saveLayersButton.Size = new System.Drawing.Size(136, 37);
            this.saveLayersButton.TabIndex = 20;
            this.saveLayersButton.Text = "Save Layers";
            this.saveLayersButton.UseVisualStyleBackColor = true;
            this.saveLayersButton.Click += new System.EventHandler(this.saveLayersButton_Click);
            // 
            // textureSynthesisButton
            // 
            this.textureSynthesisButton.Location = new System.Drawing.Point(457, 526);
            this.textureSynthesisButton.Name = "textureSynthesisButton";
            this.textureSynthesisButton.Size = new System.Drawing.Size(83, 34);
            this.textureSynthesisButton.TabIndex = 21;
            this.textureSynthesisButton.Text = "texture";
            this.textureSynthesisButton.UseVisualStyleBackColor = true;
            this.textureSynthesisButton.Click += new System.EventHandler(this.textureSynthesisButton_Click);
            // 
            // autoBox
            // 
            this.autoBox.AutoSize = true;
            this.autoBox.Location = new System.Drawing.Point(137, 653);
            this.autoBox.Name = "autoBox";
            this.autoBox.Size = new System.Drawing.Size(160, 23);
            this.autoBox.TabIndex = 22;
            this.autoBox.Text = "AutoCorrectPalette";
            this.autoBox.UseVisualStyleBackColor = true;
            // 
            // textureByLayerButton
            // 
            this.textureByLayerButton.Font = new System.Drawing.Font("Calibri", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.textureByLayerButton.Location = new System.Drawing.Point(457, 567);
            this.textureByLayerButton.Name = "textureByLayerButton";
            this.textureByLayerButton.Size = new System.Drawing.Size(83, 36);
            this.textureByLayerButton.TabIndex = 23;
            this.textureByLayerButton.Text = "texture by layers";
            this.textureByLayerButton.UseVisualStyleBackColor = true;
            this.textureByLayerButton.Click += new System.EventHandler(this.textureByLayerButton_Click);
            // 
            // getRecoloringsButton
            // 
            this.getRecoloringsButton.Location = new System.Drawing.Point(315, 609);
            this.getRecoloringsButton.Name = "getRecoloringsButton";
            this.getRecoloringsButton.Size = new System.Drawing.Size(135, 35);
            this.getRecoloringsButton.TabIndex = 25;
            this.getRecoloringsButton.Text = "Get Recolorings";
            this.getRecoloringsButton.UseVisualStyleBackColor = true;
            this.getRecoloringsButton.Click += new System.EventHandler(this.getRecoloringsButton_Click);
            // 
            // outputMeshes
            // 
            this.outputMeshes.Location = new System.Drawing.Point(194, 756);
            this.outputMeshes.Name = "outputMeshes";
            this.outputMeshes.Size = new System.Drawing.Size(231, 37);
            this.outputMeshes.TabIndex = 26;
            this.outputMeshes.Text = "Output Training Meshes";
            this.outputMeshes.UseVisualStyleBackColor = true;
            this.outputMeshes.Click += new System.EventHandler(this.outputMeshes_Click);
            // 
            // trainModelButton
            // 
            this.trainModelButton.Location = new System.Drawing.Point(431, 755);
            this.trainModelButton.Name = "trainModelButton";
            this.trainModelButton.Size = new System.Drawing.Size(126, 38);
            this.trainModelButton.TabIndex = 27;
            this.trainModelButton.Text = "Train";
            this.trainModelButton.UseVisualStyleBackColor = true;
            this.trainModelButton.Click += new System.EventHandler(this.trainModelButton_Click);
            // 
            // buttonVideo
            // 
            this.buttonVideo.Font = new System.Drawing.Font("Calibri", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.buttonVideo.Location = new System.Drawing.Point(457, 610);
            this.buttonVideo.Name = "buttonVideo";
            this.buttonVideo.Size = new System.Drawing.Size(83, 33);
            this.buttonVideo.TabIndex = 23;
            this.buttonVideo.Text = "Video";
            this.buttonVideo.UseVisualStyleBackColor = true;
            this.buttonVideo.Click += new System.EventHandler(this.buttonVideo_Click);
            // 
            // MainWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 19F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1328, 800);
            this.Controls.Add(this.trainModelButton);
            this.Controls.Add(this.outputMeshes);
            this.Controls.Add(this.getRecoloringsButton);
            this.Controls.Add(this.buttonVideo);
            this.Controls.Add(this.textureByLayerButton);
            this.Controls.Add(this.autoBox);
            this.Controls.Add(this.textureSynthesisButton);
            this.Controls.Add(this.saveLayersButton);
            this.Controls.Add(this.layerSynthesisButton);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.pictureBox);
            this.Controls.Add(this.buttonLoadConstraints);
            this.Controls.Add(this.buttonSaveConstraints);
            this.Controls.Add(this.colorSpaceBox);
            this.Controls.Add(this.colorSpaceLabel);
            this.Controls.Add(this.statusBox);
            this.Controls.Add(this.pictureBoxOriginal);
            this.Controls.Add(this.extractPaletteButton);
            this.Controls.Add(this.layerMethodBox);
            this.Controls.Add(this.layerLabel);
            this.Controls.Add(this.paletteLabel);
            this.Controls.Add(this.paletteMethodBox);
            this.Controls.Add(this.resetImageButton);
            this.Controls.Add(this.KBox);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.layerBox);
            this.Controls.Add(this.saveButton);
            this.Controls.Add(this.extractLayersButton);
            this.Controls.Add(this.openButton);
            this.Controls.Add(this.palettePanel);
            this.Cursor = System.Windows.Forms.Cursors.Default;
            this.Font = new System.Drawing.Font("Calibri", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "MainWindow";
            this.Text = "Main Window";
            this.Load += new System.EventHandler(this.MainWindow_Load);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.layerBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxOriginal)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox pictureBox;
        private System.Windows.Forms.ColorDialog colorPicker;
        private System.Windows.Forms.Panel palettePanel;
        private System.Windows.Forms.Button openButton;
        private System.Windows.Forms.Button extractLayersButton;
        private System.Windows.Forms.PictureBox layerBox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox KBox;
        private System.Windows.Forms.Button saveButton;
        private System.Windows.Forms.Button resetImageButton;
        private System.Windows.Forms.ComboBox paletteMethodBox;
        private System.Windows.Forms.Label paletteLabel;
        private System.Windows.Forms.Label layerLabel;
        private System.Windows.Forms.ComboBox layerMethodBox;
        private System.Windows.Forms.Button extractPaletteButton;
        private System.Windows.Forms.PictureBox pictureBoxOriginal;
        private System.Windows.Forms.Label colorSpaceLabel;
        private System.Windows.Forms.ComboBox colorSpaceBox;
        private System.Windows.Forms.Button buttonSaveConstraints;
        private System.Windows.Forms.Button buttonLoadConstraints;
        private System.Windows.Forms.TextBox statusBox;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button layerSynthesisButton;
        private System.Windows.Forms.Button saveLayersButton;
        private System.Windows.Forms.Button textureSynthesisButton;
        private System.Windows.Forms.CheckBox autoBox;
        private System.Windows.Forms.Button textureByLayerButton;
        private System.Windows.Forms.Button getRecoloringsButton;
        private System.Windows.Forms.Button outputMeshes;
        private System.Windows.Forms.Button trainModelButton;
        private System.Windows.Forms.Button buttonVideo;
    }
}

