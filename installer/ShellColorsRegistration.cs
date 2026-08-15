using System;
using System.Drawing;
using System.IO;
using System.Net;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using Microsoft.Win32;

internal static class RegistrationProgram
{
    private const string Version = "1.2.2";
    private const string RegistryPath = @"SOFTWARE\AuthorityGate\ShellColors";
    private const string Endpoint = "https://license.authoritygate.com/api/applications/check-in";

    [STAThread]
    private static void Main()
    {
        bool created;
        using (Mutex instance = new Mutex(true, @"Local\AuthorityGateShellColorsRegistration", out created))
        {
            if (!created) return;
            Run();
        }
    }

    private static void Run()
    {
        try
        {
            string email = Read("RegistrationEmail");
            if (String.IsNullOrWhiteSpace(email))
            {
                Application.EnableVisualStyles(); Application.SetCompatibleTextRenderingDefault(false); Application.Run(new RegistrationForm());
                return;
            }
            CheckIn(email, Read("StartingVersion"), "startup", false);
        }
        catch { }
    }

    internal static bool Register(string email, string company, out string message)
    {
        try
        {
            string starting = Read("StartingVersion"); if (String.IsNullOrEmpty(starting)) starting = Version;
            CheckIn(email, company, starting, "startup", true);
            using (RegistryKey key = Registry.CurrentUser.CreateSubKey(RegistryPath))
            {
                key.SetValue("RegistrationEmail", email.Trim()); key.SetValue("RegistrationCompany", company.Trim()); key.SetValue("StartingVersion", starting);
            }
            message = "Registration complete."; return true;
        }
        catch (WebException error)
        {
            HttpWebResponse response = error.Response as HttpWebResponse;
            if (response != null) { message = "Registration could not be recorded. Check the email address and try again, or choose Not now to continue."; return false; }
            SaveForRetry(email, company); message = "Registration was saved. ShellColors will confirm it automatically when a connection is available."; return true;
        }
        catch { SaveForRetry(email, company); message = "Registration was saved. ShellColors will confirm it automatically when a connection is available."; return true; }
    }

    private static void SaveForRetry(string email, string company)
    {
        using (RegistryKey key = Registry.CurrentUser.CreateSubKey(RegistryPath))
        {
            key.SetValue("RegistrationEmail", email.Trim()); key.SetValue("RegistrationCompany", company.Trim());
            if (key.GetValue("StartingVersion") == null) key.SetValue("StartingVersion", Version);
        }
    }

    private static string Read(string name)
    {
        using (RegistryKey key = Registry.CurrentUser.OpenSubKey(RegistryPath)) return key == null ? "" : Convert.ToString(key.GetValue(name, ""));
    }

    private static void CheckIn(string email, string startingVersion, string eventName, bool requireSuccess)
    {
        CheckIn(email, Read("RegistrationCompany"), startingVersion, eventName, requireSuccess);
    }

    private static void CheckIn(string email, string company, string startingVersion, string eventName, bool requireSuccess)
    {
        ServicePointManager.SecurityProtocol = SecurityProtocolType.Tls12;
        string json = "{\"email\":\"" + Escape(email.Trim()) + "\",\"registration_name\":\"" + Escape(company.Trim()) +
            "\",\"product\":\"cmdhelp\",\"computer_name\":\"" + Escape(Environment.MachineName) + "\",\"starting_version\":\"" +
            Escape(String.IsNullOrEmpty(startingVersion) ? Version : startingVersion) + "\",\"current_version\":\"" + Version + "\",\"event\":\"" + eventName + "\"}";
        byte[] body = Encoding.UTF8.GetBytes(json);
        HttpWebRequest request = (HttpWebRequest)WebRequest.Create(Endpoint); request.Method = "POST"; request.ContentType = "application/json";
        request.UserAgent = "AuthorityGate-ShellColors/" + Version; request.Timeout = 3500; request.ReadWriteTimeout = 3500; request.ContentLength = body.Length;
        using (Stream stream = request.GetRequestStream()) stream.Write(body, 0, body.Length);
        using (HttpWebResponse response = (HttpWebResponse)request.GetResponse())
        {
            if (requireSuccess && (int)response.StatusCode >= 300) throw new WebException("Registration failed.");
        }
    }

    private static string Escape(string value) { return value.Replace("\\", "\\\\").Replace("\"", "\\\"").Replace("\r", "").Replace("\n", "\\n"); }
}

internal sealed class RegistrationForm : Form
{
    private readonly TextBox email = new TextBox(); private readonly TextBox company = new TextBox(); private readonly Button register = new Button();
    internal RegistrationForm()
    {
        Text = "Register AuthorityGate ShellColors"; ClientSize = new Size(520, 330); FormBorderStyle = FormBorderStyle.FixedDialog; MaximizeBox = false; MinimizeBox = false;
        StartPosition = FormStartPosition.CenterScreen; Font = new Font("Segoe UI", 9F); BackColor = Color.White;
        Label title = new Label { Text = "Free ShellColors registration", Font = new Font("Segoe UI Semibold", 18F), ForeColor = Color.FromArgb(105, 28, 38), AutoSize = true, Location = new Point(28, 24) };
        Label detail = new Label { Text = "Enter your email to record this computer, installed version, and last-used date on your AuthorityGate dashboard. Company is optional. Choose Not now to continue and ShellColors will ask again later.", AutoSize = false, Location = new Point(30, 69), Size = new Size(455, 54) };
        Label emailLabel = new Label { Text = "Email address (required)", AutoSize = true, Location = new Point(30, 130) }; email.SetBounds(30, 151, 455, 27);
        Label companyLabel = new Label { Text = "Company (optional)", AutoSize = true, Location = new Point(30, 188) }; company.SetBounds(30, 209, 455, 27);
        register.Text = "Register ShellColors"; register.SetBounds(280, 267, 205, 38); register.BackColor = Color.FromArgb(105, 28, 38); register.ForeColor = Color.White; register.FlatStyle = FlatStyle.Flat; register.Click += Register;
        Button cancel = new Button { Text = "Not now", DialogResult = DialogResult.Cancel }; cancel.SetBounds(175, 267, 96, 38); cancel.Click += delegate { Close(); };
        Controls.AddRange(new Control[] { title, detail, emailLabel, email, companyLabel, company, cancel, register }); AcceptButton = register; CancelButton = cancel;
    }

    private void Register(object sender, EventArgs e)
    {
        if (String.IsNullOrWhiteSpace(email.Text) || !email.Text.Contains("@")) { MessageBox.Show("Enter a valid email address or choose Not now.", Text, MessageBoxButtons.OK, MessageBoxIcon.Warning); return; }
        register.Enabled = false; string message;
        if (RegistrationProgram.Register(email.Text, company.Text, out message)) { MessageBox.Show(message, Text, MessageBoxButtons.OK, MessageBoxIcon.Information); Close(); }
        else { MessageBox.Show(message, Text, MessageBoxButtons.OK, MessageBoxIcon.Warning); register.Enabled = true; }
    }
}
