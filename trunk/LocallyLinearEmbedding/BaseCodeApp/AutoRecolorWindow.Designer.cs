﻿namespace BaseCodeApp
{
    partial class AutoRecolorWindow
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
            this.outputMeshesButtom = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // outputMeshesButtom
            // 
            this.outputMeshesButtom.Location = new System.Drawing.Point(12, 436);
            this.outputMeshesButtom.Name = "outputMeshesButtom";
            this.outputMeshesButtom.Size = new System.Drawing.Size(104, 40);
            this.outputMeshesButtom.TabIndex = 0;
            this.outputMeshesButtom.Text = "OutputMeshes";
            this.outputMeshesButtom.UseVisualStyleBackColor = true;
            // 
            // AutoRecolorWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(629, 488);
            this.Controls.Add(this.outputMeshesButtom);
            this.Name = "AutoRecolorWindow";
            this.Text = "AutoRecolorWindow";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button outputMeshesButtom;
    }
}