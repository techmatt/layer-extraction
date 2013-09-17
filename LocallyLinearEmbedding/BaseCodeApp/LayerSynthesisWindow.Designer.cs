namespace BaseCodeApp
{
    partial class LayerSynthesisWindow
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
            this.ABox = new System.Windows.Forms.PictureBox();
            this.BBox = new System.Windows.Forms.PictureBox();
            this.ApBox = new System.Windows.Forms.PictureBox();
            this.BpBox = new System.Windows.Forms.PictureBox();
            this.resultBox = new System.Windows.Forms.PictureBox();
            this.exampleBox = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.ABox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.BBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.ApBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.BpBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.resultBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.exampleBox)).BeginInit();
            this.SuspendLayout();
            // 
            // ABox
            // 
            this.ABox.Location = new System.Drawing.Point(47, 43);
            this.ABox.Name = "ABox";
            this.ABox.Size = new System.Drawing.Size(287, 232);
            this.ABox.TabIndex = 0;
            this.ABox.TabStop = false;
            // 
            // BBox
            // 
            this.BBox.Location = new System.Drawing.Point(352, 43);
            this.BBox.Name = "BBox";
            this.BBox.Size = new System.Drawing.Size(287, 232);
            this.BBox.TabIndex = 1;
            this.BBox.TabStop = false;
            // 
            // ApBox
            // 
            this.ApBox.Location = new System.Drawing.Point(47, 291);
            this.ApBox.Name = "ApBox";
            this.ApBox.Size = new System.Drawing.Size(287, 232);
            this.ApBox.TabIndex = 2;
            this.ApBox.TabStop = false;
            // 
            // BpBox
            // 
            this.BpBox.Location = new System.Drawing.Point(352, 291);
            this.BpBox.Name = "BpBox";
            this.BpBox.Size = new System.Drawing.Size(287, 232);
            this.BpBox.TabIndex = 3;
            this.BpBox.TabStop = false;
            // 
            // resultBox
            // 
            this.resultBox.Location = new System.Drawing.Point(751, 43);
            this.resultBox.Name = "resultBox";
            this.resultBox.Size = new System.Drawing.Size(287, 232);
            this.resultBox.TabIndex = 4;
            this.resultBox.TabStop = false;
            // 
            // exampleBox
            // 
            this.exampleBox.Location = new System.Drawing.Point(751, 291);
            this.exampleBox.Name = "exampleBox";
            this.exampleBox.Size = new System.Drawing.Size(287, 232);
            this.exampleBox.TabIndex = 5;
            this.exampleBox.TabStop = false;
            // 
            // LayerSynthesisWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1079, 608);
            this.Controls.Add(this.exampleBox);
            this.Controls.Add(this.resultBox);
            this.Controls.Add(this.BpBox);
            this.Controls.Add(this.ApBox);
            this.Controls.Add(this.BBox);
            this.Controls.Add(this.ABox);
            this.Name = "LayerSynthesisWindow";
            this.Text = "LayerSynthesisWindow";
            ((System.ComponentModel.ISupportInitialize)(this.ABox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.BBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.ApBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.BpBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.resultBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.exampleBox)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.PictureBox ABox;
        private System.Windows.Forms.PictureBox BBox;
        private System.Windows.Forms.PictureBox ApBox;
        private System.Windows.Forms.PictureBox BpBox;
        private System.Windows.Forms.PictureBox resultBox;
        private System.Windows.Forms.PictureBox exampleBox;
    }
}