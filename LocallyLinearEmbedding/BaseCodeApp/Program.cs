using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace BaseCodeApp
{
    [Serializable()]
    public class EarlyAbortException : System.Exception
    {
        public EarlyAbortException() : base() { }
        public EarlyAbortException(string message) : base(message) { }
        public EarlyAbortException(string message, System.Exception inner) : base(message, inner) { }

        // A constructor is needed for serialization when an 
        // exception propagates from a remoting server to the client.  
        protected EarlyAbortException(System.Runtime.Serialization.SerializationInfo info,
            System.Runtime.Serialization.StreamingContext context) { }
    }

    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            MainWindow window;
            try
            {
                window = new MainWindow();
            }
            catch (EarlyAbortException ex)
            {
                return;
            }

            Application.Run(window);
        }
    }
}
