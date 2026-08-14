using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Reflection;
using System.Security.Principal;
using System.Threading;
using System.Windows.Forms;
using Microsoft.Win32;

internal static class Program
{
    internal const string Version = "1.1.1";
    internal static readonly string InstallDirectory = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles), "AuthorityGate", "CMDHelper");

    [STAThread]
    private static void Main(string[] args)
    {
        if (args.Length > 0 && args[0].Equals("/uninstall", StringComparison.OrdinalIgnoreCase))
        {
            EnsureElevated("/uninstall");
            Uninstall();
            return;
        }
        if (!IsAdministrator()) { EnsureElevated(""); return; }
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        Application.Run(new SetupWizard());
    }

    private static bool IsAdministrator()
    {
        return new WindowsPrincipal(WindowsIdentity.GetCurrent()).IsInRole(WindowsBuiltInRole.Administrator);
    }

    private static void EnsureElevated(string arguments)
    {
        if (IsAdministrator()) return;
        try { Process.Start(new ProcessStartInfo(Process.GetCurrentProcess().MainModule.FileName, arguments) { Verb = "runas", UseShellExecute = true }); }
        catch { }
    }

    internal static void Install()
    {
        Directory.CreateDirectory(InstallDirectory);
        Extract("CMDHelpApplication", Path.Combine(InstallDirectory, "CmdHelper.exe"));
        Extract("CMDHelpIcon", Path.Combine(InstallDirectory, "Authority_Gate_CMD.ico"));
        File.Copy(Process.GetCurrentProcess().MainModule.FileName, Path.Combine(InstallDirectory, "CMDHelpSetup.exe"), true);

        using (Process configure = Process.Start(new ProcessStartInfo(Path.Combine(InstallDirectory, "CmdHelper.exe"), "--reinstall") { UseShellExecute = true, WorkingDirectory = InstallDirectory }))
            configure.WaitForExit();

        using (RegistryKey key = Registry.LocalMachine.CreateSubKey(@"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\AuthorityGate CMD Help"))
        {
            key.SetValue("DisplayName", "AuthorityGate CMD Help");
            key.SetValue("DisplayVersion", Version);
            key.SetValue("Publisher", "AuthorityGate Inc.");
            key.SetValue("InstallLocation", InstallDirectory);
            key.SetValue("DisplayIcon", Path.Combine(InstallDirectory, "CmdHelper.exe"));
            key.SetValue("UninstallString", "\"" + Path.Combine(InstallDirectory, "CMDHelpSetup.exe") + "\" /uninstall");
            key.SetValue("NoModify", 1, RegistryValueKind.DWord);
            key.SetValue("NoRepair", 1, RegistryValueKind.DWord);
        }
    }

    private static void Extract(string resourceName, string destination)
    {
        using (Stream source = Assembly.GetExecutingAssembly().GetManifestResourceStream(resourceName))
        using (FileStream target = File.Create(destination))
            source.CopyTo(target);
    }

    private static void Uninstall()
    {
        if (MessageBox.Show("Remove AuthorityGate CMD Help and its console shortcuts?", "CMD Help Setup", MessageBoxButtons.YesNo, MessageBoxIcon.Question) != DialogResult.Yes) return;
        string app = Path.Combine(InstallDirectory, "CmdHelper.exe");
        if (File.Exists(app))
        {
            using (Process process = Process.Start(new ProcessStartInfo(app, "--uninstall") { UseShellExecute = true, WorkingDirectory = InstallDirectory }))
                process.WaitForExit();
        }
        Registry.LocalMachine.DeleteSubKeyTree(@"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\AuthorityGate CMD Help", false);
        string directory = InstallDirectory;
        Process.Start(new ProcessStartInfo("cmd.exe", "/c ping 127.0.0.1 -n 3 > nul & rmdir /s /q \"" + directory + "\"") { CreateNoWindow = true, UseShellExecute = false });
        MessageBox.Show("CMD Help was removed.", "CMD Help Setup", MessageBoxButtons.OK, MessageBoxIcon.Information);
    }
}

internal sealed class SetupWizard : Form
{
    private readonly Label heading = new Label();
    private readonly Label body = new Label();
    private readonly Label step = new Label();
    private readonly Button back = new Button();
    private readonly Button next = new Button();
    private readonly Button cancel = new Button();
    private readonly ProgressBar progress = new ProgressBar();
    private readonly CheckBox launch = new CheckBox();
    private int page;

    internal SetupWizard()
    {
        Text = "AuthorityGate CMD Help Setup";
        ClientSize = new Size(640, 430);
        FormBorderStyle = FormBorderStyle.FixedDialog;
        MaximizeBox = false;
        MinimizeBox = false;
        StartPosition = FormStartPosition.CenterScreen;
        BackColor = Color.White;
        Font = new Font("Segoe UI", 9F);

        Panel banner = new Panel { Dock = DockStyle.Top, Height = 92, BackColor = Color.FromArgb(105, 28, 38) };
        heading.SetBounds(30, 22, 570, 34); heading.Font = new Font("Segoe UI Semibold", 19F); heading.ForeColor = Color.White;
        step.SetBounds(32, 60, 560, 20); step.ForeColor = Color.FromArgb(242, 211, 141);
        banner.Controls.Add(heading); banner.Controls.Add(step); Controls.Add(banner);

        body.SetBounds(38, 125, 560, 180); body.Font = new Font("Segoe UI", 10F); body.AutoSize = false;
        progress.SetBounds(38, 285, 560, 22); progress.Style = ProgressBarStyle.Marquee; progress.Visible = false;
        launch.SetBounds(38, 286, 560, 25); launch.Text = "Launch CMD Help after setup"; launch.Checked = true; launch.Visible = false;
        Controls.Add(body); Controls.Add(progress); Controls.Add(launch);

        Panel footer = new Panel { Dock = DockStyle.Bottom, Height = 72, BackColor = Color.FromArgb(247, 247, 247) };
        back.SetBounds(330, 19, 88, 34); back.Text = "< Back"; back.Click += (s, e) => { if (page > 0) { page--; Render(); } };
        next.SetBounds(426, 19, 88, 34); next.Text = "Next >"; next.Click += Next;
        cancel.SetBounds(522, 19, 88, 34); cancel.Text = "Cancel"; cancel.Click += (s, e) => Close();
        footer.Controls.Add(back); footer.Controls.Add(next); footer.Controls.Add(cancel); Controls.Add(footer);
        AcceptButton = next; CancelButton = cancel;
        Render();
    }

    private void Render()
    {
        back.Enabled = page > 0 && page < 3;
        progress.Visible = false; launch.Visible = false;
        if (page == 0)
        {
            heading.Text = "Welcome to CMD Help Setup"; step.Text = "AuthorityGate CMD Help " + Program.Version;
            body.Text = "This wizard installs the newest CMD Help release.\r\n\r\nCMD Help restores separate Windows 11 color identities for PowerShell, Administrator, User, and System console windows.\r\n\r\nClick Next to continue.";
            next.Text = "Next >";
        }
        else if (page == 1)
        {
            heading.Text = "Ready to install"; step.Text = "Installation folder";
            body.Text = "Setup will install or upgrade CMD Help in:\r\n\r\n" + Program.InstallDirectory + "\r\n\r\nExplorer context-menu entries, Start Menu shortcuts, and Windows uninstall information will be configured.";
            next.Text = "Install";
        }
        else
        {
            heading.Text = "Setup complete"; step.Text = "CMD Help " + Program.Version + " is installed";
            body.Text = "The newest CMD Help version was installed successfully.\r\n\r\nUse the AuthorityGate Utilities Start Menu folder or right-click a folder to open a color-coded console.";
            launch.Visible = true; next.Text = "Finish"; cancel.Enabled = false;
        }
    }

    private async void Next(object sender, EventArgs e)
    {
        if (page == 0) { page = 1; Render(); return; }
        if (page == 1)
        {
            page = 3; heading.Text = "Installing CMD Help"; step.Text = "Please wait"; body.Text = "Installing the signed application and configuring Windows integration…";
            progress.Visible = true; back.Enabled = false; next.Enabled = false; cancel.Enabled = false;
            try
            {
                await System.Threading.Tasks.Task.Run((Action)Program.Install);
                page = 2; next.Enabled = true; cancel.Enabled = true; Render();
            }
            catch (Exception error)
            {
                MessageBox.Show("Setup could not complete.\r\n\r\n" + error.Message, "CMD Help Setup", MessageBoxButtons.OK, MessageBoxIcon.Error);
                page = 1; next.Enabled = true; cancel.Enabled = true; Render();
            }
            return;
        }
        if (page == 2)
        {
            if (launch.Checked) Process.Start(new ProcessStartInfo(Path.Combine(Program.InstallDirectory, "CmdHelper.exe"), "--user") { UseShellExecute = true, WorkingDirectory = Program.InstallDirectory });
            Close();
        }
    }
}
