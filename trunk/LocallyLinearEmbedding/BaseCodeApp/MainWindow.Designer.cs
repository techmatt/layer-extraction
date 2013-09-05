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
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.layerBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxOriginal)).BeginInit();
            this.SuspendLayout();
            // 
            // pictureBox
            // 
            this.pictureBox.Location = new System.Drawing.Point(26, 37);
            this.pictureBox.Name = "pictureBox";
            this.pictureBox.Size = new System.Drawing.Size(562, 465);
            this.pictureBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pictureBox.TabIndex = 0;
            this.pictureBox.TabStop = false;
            // 
            // palettePanel
            // 
            this.palettePanel.Location = new System.Drawing.Point(617, 37);
            this.palettePanel.Name = "palettePanel";
            this.palettePanel.Size = new System.Drawing.Size(169, 464);
            this.palettePanel.TabIndex = 1;
            // 
            // openButton
            // 
            this.openButton.Location = new System.Drawing.Point(28, 511);
            this.openButton.Name = "openButton";
            this.openButton.Size = new System.Drawing.Size(103, 35);
            this.openButton.TabIndex = 2;
            this.openButton.Text = "Open Image";
            this.openButton.UseVisualStyleBackColor = true;
            this.openButton.Click += new System.EventHandler(this.openButton_Click);
            // 
            // extractLayersButton
            // 
            this.extractLayersButton.Location = new System.Drawing.Point(137, 553);
            this.extractLayersButton.Name = "extractLayersButton";
            this.extractLayersButton.Size = new System.Drawing.Size(136, 35);
            this.extractLayersButton.TabIndex = 3;
            this.extractLayersButton.Text = "Extract Layers";
            this.extractLayersButton.UseVisualStyleBackColor = true;
            this.extractLayersButton.Click += new System.EventHandler(this.extractLayersButton_Click);
            // 
            // layerBox
            // 
            this.layerBox.Location = new System.Drawing.Point(811, 37);
            this.layerBox.Name = "layerBox";
            this.layerBox.Size = new System.Drawing.Size(349, 241);
            this.layerBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.layerBox.TabIndex = 4;
            this.layerBox.TabStop = false;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(289, 519);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(30, 19);
            this.label1.TabIndex = 5;
            this.label1.Text = "K =";
            // 
            // KBox
            // 
            this.KBox.Location = new System.Drawing.Point(325, 515);
            this.KBox.Name = "KBox";
            this.KBox.Size = new System.Drawing.Size(72, 27);
            this.KBox.TabIndex = 6;
            this.KBox.Text = "5";
            // 
            // saveButton
            // 
            this.saveButton.Location = new System.Drawing.Point(279, 552);
            this.saveButton.Name = "saveButton";
            this.saveButton.Size = new System.Drawing.Size(136, 35);
            this.saveButton.TabIndex = 3;
            this.saveButton.Text = "Save Image";
            this.saveButton.UseVisualStyleBackColor = true;
            this.saveButton.Click += new System.EventHandler(this.saveButton_Click);
            // 
            // resetImageButton
            // 
            this.resetImageButton.Location = new System.Drawing.Point(28, 552);
            this.resetImageButton.Name = "resetImageButton";
            this.resetImageButton.Size = new System.Drawing.Size(103, 37);
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
            "CHI-Resize"});
            this.paletteMethodBox.Location = new System.Drawing.Point(530, 515);
            this.paletteMethodBox.Name = "paletteMethodBox";
            this.paletteMethodBox.Size = new System.Drawing.Size(162, 27);
            this.paletteMethodBox.TabIndex = 8;
            // 
            // paletteLabel
            // 
            this.paletteLabel.AutoSize = true;
            this.paletteLabel.Location = new System.Drawing.Point(458, 517);
            this.paletteLabel.Name = "paletteLabel";
            this.paletteLabel.Size = new System.Drawing.Size(57, 19);
            this.paletteLabel.TabIndex = 9;
            this.paletteLabel.Text = "Palette";
            // 
            // layerLabel
            // 
            this.layerLabel.AutoSize = true;
            this.layerLabel.Location = new System.Drawing.Point(421, 561);
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
            this.layerMethodBox.Location = new System.Drawing.Point(530, 558);
            this.layerMethodBox.Name = "layerMethodBox";
            this.layerMethodBox.Size = new System.Drawing.Size(162, 27);
            this.layerMethodBox.TabIndex = 11;
            // 
            // extractPaletteButton
            // 
            this.extractPaletteButton.Location = new System.Drawing.Point(138, 511);
            this.extractPaletteButton.Name = "extractPaletteButton";
            this.extractPaletteButton.Size = new System.Drawing.Size(135, 35);
            this.extractPaletteButton.TabIndex = 12;
            this.extractPaletteButton.Text = "Extract Palette";
            this.extractPaletteButton.UseVisualStyleBackColor = true;
            this.extractPaletteButton.Click += new System.EventHandler(this.extractPaletteButton_Click);
            // 
            // pictureBoxOriginal
            // 
            this.pictureBoxOriginal.Location = new System.Drawing.Point(812, 289);
            this.pictureBoxOriginal.Name = "pictureBoxOriginal";
            this.pictureBoxOriginal.Size = new System.Drawing.Size(347, 212);
            this.pictureBoxOriginal.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pictureBoxOriginal.TabIndex = 13;
            this.pictureBoxOriginal.TabStop = false;
            // 
            // MainWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 19F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1225, 625);
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
            this.Controls.Add(this.pictureBox);
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
    }
}

