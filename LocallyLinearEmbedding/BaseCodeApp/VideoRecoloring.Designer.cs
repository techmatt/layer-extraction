﻿namespace BaseCodeApp
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
            this.previewBox = new System.Windows.Forms.PictureBox();
            this.videoBox = new System.Windows.Forms.PictureBox();
            this.palettePanel = new System.Windows.Forms.Panel();
            this.openButton = new System.Windows.Forms.Button();
            this.resetButton = new System.Windows.Forms.Button();
            this.fpsBox = new System.Windows.Forms.ComboBox();
            this.fpsLabel = new System.Windows.Forms.Label();
            this.KBox = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.timerVideoFrame = new System.Windows.Forms.Timer(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.previewBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.videoBox)).BeginInit();
            this.SuspendLayout();
            // 
            // previewBox
            // 
            this.previewBox.Location = new System.Drawing.Point(12, 19);
            this.previewBox.Name = "previewBox";
            this.previewBox.Size = new System.Drawing.Size(386, 248);
            this.previewBox.TabIndex = 0;
            this.previewBox.TabStop = false;
            // 
            // videoBox
            // 
            this.videoBox.Location = new System.Drawing.Point(418, 19);
            this.videoBox.Name = "videoBox";
            this.videoBox.Size = new System.Drawing.Size(622, 456);
            this.videoBox.TabIndex = 1;
            this.videoBox.TabStop = false;
            // 
            // palettePanel
            // 
            this.palettePanel.Location = new System.Drawing.Point(12, 293);
            this.palettePanel.Name = "palettePanel";
            this.palettePanel.Size = new System.Drawing.Size(385, 181);
            this.palettePanel.TabIndex = 2;
            // 
            // openButton
            // 
            this.openButton.Font = new System.Drawing.Font("Calibri", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.openButton.Location = new System.Drawing.Point(12, 492);
            this.openButton.Name = "openButton";
            this.openButton.Size = new System.Drawing.Size(152, 42);
            this.openButton.TabIndex = 3;
            this.openButton.Text = "Load Video";
            this.openButton.UseVisualStyleBackColor = true;
            this.openButton.Click += new System.EventHandler(this.openButton_Click);
            // 
            // resetButton
            // 
            this.resetButton.Font = new System.Drawing.Font("Calibri", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.resetButton.Location = new System.Drawing.Point(12, 551);
            this.resetButton.Name = "resetButton";
            this.resetButton.Size = new System.Drawing.Size(152, 42);
            this.resetButton.TabIndex = 4;
            this.resetButton.Text = "Reset";
            this.resetButton.UseVisualStyleBackColor = true;
            // 
            // fpsBox
            // 
            this.fpsBox.Font = new System.Drawing.Font("Calibri", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.fpsBox.FormattingEnabled = true;
            this.fpsBox.Location = new System.Drawing.Point(211, 501);
            this.fpsBox.Name = "fpsBox";
            this.fpsBox.Size = new System.Drawing.Size(66, 27);
            this.fpsBox.TabIndex = 5;
            // 
            // fpsLabel
            // 
            this.fpsLabel.AutoSize = true;
            this.fpsLabel.Font = new System.Drawing.Font("Calibri", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.fpsLabel.Location = new System.Drawing.Point(172, 504);
            this.fpsLabel.Name = "fpsLabel";
            this.fpsLabel.Size = new System.Drawing.Size(33, 19);
            this.fpsLabel.TabIndex = 6;
            this.fpsLabel.Text = "fps:";
            // 
            // KBox
            // 
            this.KBox.Font = new System.Drawing.Font("Calibri", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.KBox.Location = new System.Drawing.Point(329, 501);
            this.KBox.Name = "KBox";
            this.KBox.Size = new System.Drawing.Size(69, 27);
            this.KBox.TabIndex = 8;
            this.KBox.Text = "5";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Calibri", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(293, 505);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(30, 19);
            this.label1.TabIndex = 7;
            this.label1.Text = "K =";
            // 
            // timerVideoFrame
            // 
            this.timerVideoFrame.Enabled = true;
            this.timerVideoFrame.Tick += new System.EventHandler(this.timerVideoFrame_Tick);
            // 
            // VideoRecoloring
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1061, 630);
            this.Controls.Add(this.KBox);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.fpsLabel);
            this.Controls.Add(this.fpsBox);
            this.Controls.Add(this.resetButton);
            this.Controls.Add(this.openButton);
            this.Controls.Add(this.palettePanel);
            this.Controls.Add(this.videoBox);
            this.Controls.Add(this.previewBox);
            this.Name = "VideoRecoloring";
            this.Text = "VideoRecoloring";
            ((System.ComponentModel.ISupportInitialize)(this.previewBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.videoBox)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox previewBox;
        private System.Windows.Forms.PictureBox videoBox;
        private System.Windows.Forms.Panel palettePanel;
        private System.Windows.Forms.Button openButton;
        private System.Windows.Forms.Button resetButton;
        private System.Windows.Forms.ComboBox fpsBox;
        private System.Windows.Forms.Label fpsLabel;
        private System.Windows.Forms.TextBox KBox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Timer timerVideoFrame;
    }
}