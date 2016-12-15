namespace interactiveRecoloringUI
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainWindow));
            this.pictureBoxPalette = new System.Windows.Forms.PictureBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.buttonLoadImage = new System.Windows.Forms.Button();
            this.buttonReset = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.pictureBoxMain = new System.Windows.Forms.PictureBox();
            this.trackBarStationaryRadius = new System.Windows.Forms.TrackBar();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.buttonUpdate = new System.Windows.Forms.Button();
            this.pictureBoxSelectedColor = new System.Windows.Forms.PictureBox();
            this.pictureBoxResult = new System.Windows.Forms.PictureBox();
            this.colorDialog = new System.Windows.Forms.ColorDialog();
            this.checkBoxOriginal = new System.Windows.Forms.CheckBox();
            this.checkBoxShowConstraints = new System.Windows.Forms.CheckBox();
            this.buttonSave = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxPalette)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxMain)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackBarStationaryRadius)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxSelectedColor)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxResult)).BeginInit();
            this.SuspendLayout();
            // 
            // pictureBoxPalette
            // 
            this.pictureBoxPalette.Image = ((System.Drawing.Image)(resources.GetObject("pictureBoxPalette.Image")));
            this.pictureBoxPalette.InitialImage = null;
            this.pictureBoxPalette.Location = new System.Drawing.Point(10, 72);
            this.pictureBoxPalette.Name = "pictureBoxPalette";
            this.pictureBoxPalette.Size = new System.Drawing.Size(300, 300);
            this.pictureBoxPalette.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.pictureBoxPalette.TabIndex = 0;
            this.pictureBoxPalette.TabStop = false;
            this.pictureBoxPalette.Click += new System.EventHandler(this.pictureBoxPalette_Click);
            this.pictureBoxPalette.MouseDown += new System.Windows.Forms.MouseEventHandler(this.pictureBoxPalette_MouseDown);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 47);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(81, 15);
            this.label1.TabIndex = 1;
            this.label1.Text = "Color palette:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(320, 47);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(87, 15);
            this.label2.TabIndex = 1;
            this.label2.Text = "Current image:";
            // 
            // buttonLoadImage
            // 
            this.buttonLoadImage.Location = new System.Drawing.Point(10, 12);
            this.buttonLoadImage.Name = "buttonLoadImage";
            this.buttonLoadImage.Size = new System.Drawing.Size(112, 30);
            this.buttonLoadImage.TabIndex = 2;
            this.buttonLoadImage.Text = "Load image";
            this.buttonLoadImage.UseVisualStyleBackColor = true;
            this.buttonLoadImage.Click += new System.EventHandler(this.buttonLoadImage_Click);
            // 
            // buttonReset
            // 
            this.buttonReset.Location = new System.Drawing.Point(128, 12);
            this.buttonReset.Name = "buttonReset";
            this.buttonReset.Size = new System.Drawing.Size(101, 30);
            this.buttonReset.TabIndex = 3;
            this.buttonReset.Text = "Reset image";
            this.buttonReset.UseVisualStyleBackColor = true;
            this.buttonReset.Click += new System.EventHandler(this.buttonReset_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(656, 20);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(304, 15);
            this.label3.TabIndex = 1;
            this.label3.Text = "Shift-click image for color dropper.  Ctrl-click to erase.";
            this.label3.Click += new System.EventHandler(this.label3_Click);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(7, 404);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(250, 15);
            this.label4.TabIndex = 4;
            this.label4.Text = "Magenta indicates regions to keep the same.";
            // 
            // pictureBoxMain
            // 
            this.pictureBoxMain.Location = new System.Drawing.Point(323, 74);
            this.pictureBoxMain.Name = "pictureBoxMain";
            this.pictureBoxMain.Size = new System.Drawing.Size(421, 391);
            this.pictureBoxMain.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.pictureBoxMain.TabIndex = 5;
            this.pictureBoxMain.TabStop = false;
            this.pictureBoxMain.Click += new System.EventHandler(this.pictureBoxMain_Click);
            this.pictureBoxMain.MouseDown += new System.Windows.Forms.MouseEventHandler(this.pictureBoxMain_MouseDown);
            this.pictureBoxMain.MouseMove += new System.Windows.Forms.MouseEventHandler(this.pictureBoxMain_MouseMove);
            // 
            // trackBarStationaryRadius
            // 
            this.trackBarStationaryRadius.Location = new System.Drawing.Point(10, 452);
            this.trackBarStationaryRadius.Maximum = 100;
            this.trackBarStationaryRadius.Name = "trackBarStationaryRadius";
            this.trackBarStationaryRadius.Size = new System.Drawing.Size(294, 45);
            this.trackBarStationaryRadius.TabIndex = 6;
            this.trackBarStationaryRadius.TickStyle = System.Windows.Forms.TickStyle.None;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Font = new System.Drawing.Font("Calibri", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label5.Location = new System.Drawing.Point(12, 430);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(160, 19);
            this.label5.TabIndex = 7;
            this.label5.Text = "Stationary region size:";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(11, 477);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(91, 15);
            this.label6.TabIndex = 8;
            this.label6.Text = "(more changes)";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(216, 477);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(93, 15);
            this.label7.TabIndex = 8;
            this.label7.Text = "(fewer changes)";
            // 
            // buttonUpdate
            // 
            this.buttonUpdate.Location = new System.Drawing.Point(235, 12);
            this.buttonUpdate.Name = "buttonUpdate";
            this.buttonUpdate.Size = new System.Drawing.Size(101, 30);
            this.buttonUpdate.TabIndex = 3;
            this.buttonUpdate.Text = "Update (enter)";
            this.buttonUpdate.UseVisualStyleBackColor = true;
            this.buttonUpdate.Click += new System.EventHandler(this.buttonUpdate_Click);
            // 
            // pictureBoxSelectedColor
            // 
            this.pictureBoxSelectedColor.BackColor = System.Drawing.Color.Black;
            this.pictureBoxSelectedColor.Location = new System.Drawing.Point(10, 378);
            this.pictureBoxSelectedColor.Name = "pictureBoxSelectedColor";
            this.pictureBoxSelectedColor.Size = new System.Drawing.Size(299, 23);
            this.pictureBoxSelectedColor.TabIndex = 10;
            this.pictureBoxSelectedColor.TabStop = false;
            this.pictureBoxSelectedColor.Click += new System.EventHandler(this.pictureBoxSelectedColor_Click);
            // 
            // pictureBoxResult
            // 
            this.pictureBoxResult.Location = new System.Drawing.Point(323, 558);
            this.pictureBoxResult.Name = "pictureBoxResult";
            this.pictureBoxResult.Size = new System.Drawing.Size(456, 257);
            this.pictureBoxResult.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.pictureBoxResult.TabIndex = 11;
            this.pictureBoxResult.TabStop = false;
            // 
            // checkBoxOriginal
            // 
            this.checkBoxOriginal.AutoSize = true;
            this.checkBoxOriginal.Location = new System.Drawing.Point(421, 19);
            this.checkBoxOriginal.Name = "checkBoxOriginal";
            this.checkBoxOriginal.Size = new System.Drawing.Size(102, 19);
            this.checkBoxOriginal.TabIndex = 12;
            this.checkBoxOriginal.Text = "Show original";
            this.checkBoxOriginal.UseVisualStyleBackColor = true;
            this.checkBoxOriginal.CheckedChanged += new System.EventHandler(this.checkBoxOriginal_CheckedChanged);
            // 
            // checkBoxShowConstraints
            // 
            this.checkBoxShowConstraints.AutoSize = true;
            this.checkBoxShowConstraints.Checked = true;
            this.checkBoxShowConstraints.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxShowConstraints.Location = new System.Drawing.Point(529, 19);
            this.checkBoxShowConstraints.Name = "checkBoxShowConstraints";
            this.checkBoxShowConstraints.Size = new System.Drawing.Size(121, 19);
            this.checkBoxShowConstraints.TabIndex = 12;
            this.checkBoxShowConstraints.Text = "Show constraints";
            this.checkBoxShowConstraints.UseVisualStyleBackColor = true;
            this.checkBoxShowConstraints.CheckedChanged += new System.EventHandler(this.checkBoxShowConstraints_CheckedChanged);
            // 
            // buttonSave
            // 
            this.buttonSave.Location = new System.Drawing.Point(342, 12);
            this.buttonSave.Name = "buttonSave";
            this.buttonSave.Size = new System.Drawing.Size(73, 30);
            this.buttonSave.TabIndex = 3;
            this.buttonSave.Text = "Save";
            this.buttonSave.UseVisualStyleBackColor = true;
            this.buttonSave.Click += new System.EventHandler(this.buttonSave_Click);
            // 
            // MainWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1284, 1271);
            this.Controls.Add(this.checkBoxShowConstraints);
            this.Controls.Add(this.checkBoxOriginal);
            this.Controls.Add(this.pictureBoxResult);
            this.Controls.Add(this.pictureBoxSelectedColor);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.trackBarStationaryRadius);
            this.Controls.Add(this.pictureBoxMain);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.buttonSave);
            this.Controls.Add(this.buttonUpdate);
            this.Controls.Add(this.buttonReset);
            this.Controls.Add(this.buttonLoadImage);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.pictureBoxPalette);
            this.Font = new System.Drawing.Font("Calibri", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Name = "MainWindow";
            this.Text = "Interactive recoloring demo";
            this.Load += new System.EventHandler(this.MainWindow_Load);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxPalette)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxMain)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackBarStationaryRadius)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxSelectedColor)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxResult)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox pictureBoxPalette;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button buttonLoadImage;
        private System.Windows.Forms.Button buttonReset;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.PictureBox pictureBoxMain;
        private System.Windows.Forms.TrackBar trackBarStationaryRadius;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Button buttonUpdate;
        private System.Windows.Forms.PictureBox pictureBoxSelectedColor;
        private System.Windows.Forms.PictureBox pictureBoxResult;
        private System.Windows.Forms.ColorDialog colorDialog;
        private System.Windows.Forms.CheckBox checkBoxOriginal;
        private System.Windows.Forms.CheckBox checkBoxShowConstraints;
        private System.Windows.Forms.Button buttonSave;
    }
}

