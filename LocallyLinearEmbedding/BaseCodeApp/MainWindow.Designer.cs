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
            this.palettePanel = new System.Windows.Forms.Panel();
            this.openButton = new System.Windows.Forms.Button();
            this.extractLayersButton = new System.Windows.Forms.Button();
            this.layerBox = new System.Windows.Forms.PictureBox();
            this.label1 = new System.Windows.Forms.Label();
            this.KBox = new System.Windows.Forms.TextBox();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxOriginal)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.layerBox)).BeginInit();
            this.SuspendLayout();
            // 
            // pictureBoxOriginal
            // 
            this.pictureBoxOriginal.Location = new System.Drawing.Point(26, 37);
            this.pictureBoxOriginal.Name = "pictureBoxOriginal";
            this.pictureBoxOriginal.Size = new System.Drawing.Size(562, 465);
            this.pictureBoxOriginal.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pictureBoxOriginal.TabIndex = 0;
            this.pictureBoxOriginal.TabStop = false;
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
            this.extractLayersButton.Location = new System.Drawing.Point(168, 511);
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
            this.label1.Location = new System.Drawing.Point(360, 518);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(30, 19);
            this.label1.TabIndex = 5;
            this.label1.Text = "K =";
            // 
            // KBox
            // 
            this.KBox.Location = new System.Drawing.Point(396, 516);
            this.KBox.Name = "KBox";
            this.KBox.Size = new System.Drawing.Size(72, 27);
            this.KBox.TabIndex = 6;
            this.KBox.Text = "5";
            // 
            // MainWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 19F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1225, 625);
            this.Controls.Add(this.KBox);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.layerBox);
            this.Controls.Add(this.extractLayersButton);
            this.Controls.Add(this.openButton);
            this.Controls.Add(this.palettePanel);
            this.Controls.Add(this.pictureBoxOriginal);
            this.Cursor = System.Windows.Forms.Cursors.Default;
            this.Font = new System.Drawing.Font("Calibri", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "MainWindow";
            this.Text = "Main Window";
            this.Load += new System.EventHandler(this.MainWindow_Load);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxOriginal)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.layerBox)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox pictureBoxOriginal;
        private System.Windows.Forms.ColorDialog colorPicker;
        private System.Windows.Forms.Panel palettePanel;
        private System.Windows.Forms.Button openButton;
        private System.Windows.Forms.Button extractLayersButton;
        private System.Windows.Forms.PictureBox layerBox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox KBox;
    }
}

