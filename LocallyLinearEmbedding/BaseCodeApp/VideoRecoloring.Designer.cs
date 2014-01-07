namespace BaseCodeApp
{
    partial class VideoRecoloring
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
            this.components = new System.ComponentModel.Container();
            this.videoBox = new System.Windows.Forms.PictureBox();
            this.palettePanel = new System.Windows.Forms.Panel();
            this.openButton = new System.Windows.Forms.Button();
            this.resetButton = new System.Windows.Forms.Button();
            this.fpsBox = new System.Windows.Forms.ComboBox();
            this.fpsLabel = new System.Windows.Forms.Label();
            this.KBox = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.timerVideoFrame = new System.Windows.Forms.Timer(this.components);
            this.colorPicker = new System.Windows.Forms.ColorDialog();
            this.suggestButton = new System.Windows.Forms.Button();
            this.savePaletteImageButton = new System.Windows.Forms.Button();
            this.pictureBoxPalette = new System.Windows.Forms.PictureBox();
            this.pictureBoxScroll = new System.Windows.Forms.PictureBox();
            this.pictureBoxColor = new System.Windows.Forms.PictureBox();
            this.panelDecision = new System.Windows.Forms.Panel();
            this.saveButton = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.videoBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxPalette)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxScroll)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxColor)).BeginInit();
            this.SuspendLayout();
            // 
            // videoBox
            // 
            this.videoBox.Location = new System.Drawing.Point(12, 12);
            this.videoBox.Name = "videoBox";
            this.videoBox.Size = new System.Drawing.Size(1159, 463);
            this.videoBox.TabIndex = 1;
            this.videoBox.TabStop = false;
            // 
            // palettePanel
            // 
            this.palettePanel.Location = new System.Drawing.Point(434, 481);
            this.palettePanel.Name = "palettePanel";
            this.palettePanel.Size = new System.Drawing.Size(737, 200);
            this.palettePanel.TabIndex = 2;
            // 
            // openButton
            // 
            this.openButton.Font = new System.Drawing.Font("Calibri", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.openButton.Location = new System.Drawing.Point(11, 525);
            this.openButton.Name = "openButton";
            this.openButton.Size = new System.Drawing.Size(120, 37);
            this.openButton.TabIndex = 3;
            this.openButton.Text = "Load";
            this.openButton.UseVisualStyleBackColor = true;
            this.openButton.Click += new System.EventHandler(this.openButton_Click);
            // 
            // resetButton
            // 
            this.resetButton.Font = new System.Drawing.Font("Calibri", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.resetButton.Location = new System.Drawing.Point(11, 565);
            this.resetButton.Name = "resetButton";
            this.resetButton.Size = new System.Drawing.Size(120, 37);
            this.resetButton.TabIndex = 4;
            this.resetButton.Text = "Reset";
            this.resetButton.UseVisualStyleBackColor = true;
            this.resetButton.Click += new System.EventHandler(this.resetButton_Click);
            // 
            // fpsBox
            // 
            this.fpsBox.Font = new System.Drawing.Font("Calibri", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.fpsBox.FormattingEnabled = true;
            this.fpsBox.Items.AddRange(new object[] {
            "5",
            "8",
            "9",
            "10",
            "12.5",
            "15",
            "24"});
            this.fpsBox.Location = new System.Drawing.Point(846, 765);
            this.fpsBox.Name = "fpsBox";
            this.fpsBox.Size = new System.Drawing.Size(66, 27);
            this.fpsBox.TabIndex = 5;
            this.fpsBox.Visible = false;
            this.fpsBox.SelectedIndexChanged += new System.EventHandler(this.fpsBox_Changed);
            // 
            // fpsLabel
            // 
            this.fpsLabel.AutoSize = true;
            this.fpsLabel.Font = new System.Drawing.Font("Calibri", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.fpsLabel.Location = new System.Drawing.Point(807, 768);
            this.fpsLabel.Name = "fpsLabel";
            this.fpsLabel.Size = new System.Drawing.Size(33, 19);
            this.fpsLabel.TabIndex = 6;
            this.fpsLabel.Text = "fps:";
            this.fpsLabel.Visible = false;
            // 
            // KBox
            // 
            this.KBox.Font = new System.Drawing.Font("Calibri", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.KBox.Location = new System.Drawing.Point(964, 765);
            this.KBox.Name = "KBox";
            this.KBox.Size = new System.Drawing.Size(69, 27);
            this.KBox.TabIndex = 8;
            this.KBox.Text = "5";
            this.KBox.Visible = false;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Calibri", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(928, 769);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(30, 19);
            this.label1.TabIndex = 7;
            this.label1.Text = "K =";
            this.label1.Visible = false;
            // 
            // timerVideoFrame
            // 
            this.timerVideoFrame.Enabled = true;
            this.timerVideoFrame.Tick += new System.EventHandler(this.timerVideoFrame_Tick);
            // 
            // suggestButton
            // 
            this.suggestButton.Font = new System.Drawing.Font("Calibri", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.suggestButton.Location = new System.Drawing.Point(11, 605);
            this.suggestButton.Name = "suggestButton";
            this.suggestButton.Size = new System.Drawing.Size(120, 37);
            this.suggestButton.TabIndex = 9;
            this.suggestButton.Text = "Suggest";
            this.suggestButton.UseVisualStyleBackColor = true;
            this.suggestButton.Click += new System.EventHandler(this.suggestButton_Click);
            // 
            // savePaletteImageButton
            // 
            this.savePaletteImageButton.Font = new System.Drawing.Font("Calibri", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.savePaletteImageButton.Location = new System.Drawing.Point(846, 717);
            this.savePaletteImageButton.Name = "savePaletteImageButton";
            this.savePaletteImageButton.Size = new System.Drawing.Size(151, 42);
            this.savePaletteImageButton.TabIndex = 10;
            this.savePaletteImageButton.Text = "Save Palette Image";
            this.savePaletteImageButton.UseVisualStyleBackColor = true;
            this.savePaletteImageButton.Visible = false;
            this.savePaletteImageButton.Click += new System.EventHandler(this.savePaletteImageButton_Click);
            // 
            // pictureBoxPalette
            // 
            this.pictureBoxPalette.Location = new System.Drawing.Point(148, 481);
            this.pictureBoxPalette.Name = "pictureBoxPalette";
            this.pictureBoxPalette.Size = new System.Drawing.Size(200, 200);
            this.pictureBoxPalette.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.pictureBoxPalette.TabIndex = 11;
            this.pictureBoxPalette.TabStop = false;
            this.pictureBoxPalette.Click += new System.EventHandler(this.pictureBoxPalette_Click);
            this.pictureBoxPalette.MouseDown += new System.Windows.Forms.MouseEventHandler(this.pictureBoxPalette_MouseDown);
            this.pictureBoxPalette.MouseMove += new System.Windows.Forms.MouseEventHandler(this.pictureBoxPalette_MouseMove);
            this.pictureBoxPalette.MouseUp += new System.Windows.Forms.MouseEventHandler(this.pictureBoxPalette_MouseUp);
            // 
            // pictureBoxScroll
            // 
            this.pictureBoxScroll.Location = new System.Drawing.Point(364, 481);
            this.pictureBoxScroll.Name = "pictureBoxScroll";
            this.pictureBoxScroll.Size = new System.Drawing.Size(20, 200);
            this.pictureBoxScroll.TabIndex = 12;
            this.pictureBoxScroll.TabStop = false;
            this.pictureBoxScroll.Click += new System.EventHandler(this.pictureBoxScroll_Click);
            this.pictureBoxScroll.MouseDown += new System.Windows.Forms.MouseEventHandler(this.pictureBoxScroll_MouseDown);
            this.pictureBoxScroll.MouseMove += new System.Windows.Forms.MouseEventHandler(this.pictureBoxScroll_MouseMove);
            this.pictureBoxScroll.MouseUp += new System.Windows.Forms.MouseEventHandler(this.pictureBoxScroll_MouseUp);
            // 
            // pictureBoxColor
            // 
            this.pictureBoxColor.Location = new System.Drawing.Point(11, 481);
            this.pictureBoxColor.Name = "pictureBoxColor";
            this.pictureBoxColor.Size = new System.Drawing.Size(120, 40);
            this.pictureBoxColor.TabIndex = 14;
            this.pictureBoxColor.TabStop = false;
            // 
            // panelDecision
            // 
            this.panelDecision.Location = new System.Drawing.Point(12, 12);
            this.panelDecision.Name = "panelDecision";
            this.panelDecision.Size = new System.Drawing.Size(1159, 463);
            this.panelDecision.TabIndex = 15;
            // 
            // saveButton
            // 
            this.saveButton.Font = new System.Drawing.Font("Calibri", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.saveButton.Location = new System.Drawing.Point(11, 645);
            this.saveButton.Name = "saveButton";
            this.saveButton.Size = new System.Drawing.Size(120, 37);
            this.saveButton.TabIndex = 16;
            this.saveButton.Text = "Save";
            this.saveButton.UseVisualStyleBackColor = true;
            this.saveButton.Click += new System.EventHandler(this.saveButton_Click);
            // 
            // VideoRecoloring
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1183, 707);
            this.Controls.Add(this.saveButton);
            this.Controls.Add(this.panelDecision);
            this.Controls.Add(this.pictureBoxColor);
            this.Controls.Add(this.pictureBoxScroll);
            this.Controls.Add(this.pictureBoxPalette);
            this.Controls.Add(this.savePaletteImageButton);
            this.Controls.Add(this.suggestButton);
            this.Controls.Add(this.KBox);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.fpsLabel);
            this.Controls.Add(this.fpsBox);
            this.Controls.Add(this.resetButton);
            this.Controls.Add(this.openButton);
            this.Controls.Add(this.palettePanel);
            this.Controls.Add(this.videoBox);
            this.Name = "VideoRecoloring";
            this.Text = "VideoRecoloring";
            this.Load += new System.EventHandler(this.VideoRecoloring_Load);
            ((System.ComponentModel.ISupportInitialize)(this.videoBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxPalette)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxScroll)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxColor)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox videoBox;
        private System.Windows.Forms.Panel palettePanel;
        private System.Windows.Forms.Button openButton;
        private System.Windows.Forms.Button resetButton;
        private System.Windows.Forms.ComboBox fpsBox;
        private System.Windows.Forms.Label fpsLabel;
        private System.Windows.Forms.TextBox KBox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Timer timerVideoFrame;
        private System.Windows.Forms.ColorDialog colorPicker;
        private System.Windows.Forms.Button suggestButton;
        private System.Windows.Forms.Button savePaletteImageButton;
        private System.Windows.Forms.PictureBox pictureBoxPalette;
        private System.Windows.Forms.PictureBox pictureBoxScroll;
        private System.Windows.Forms.PictureBox pictureBoxColor;
        private System.Windows.Forms.Panel panelDecision;
        private System.Windows.Forms.Button saveButton;
    }
}