using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Reflection;
using System.Security.Principal;
using System.Windows.Forms;
using Microsoft.Win32;

internal static class Program
{
    internal const string Version = "1.2.0";
    internal const string ProductName = "AuthorityGate ShellColors";
    internal static readonly string InstallDirectory = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles), "AuthorityGate", "ShellColors");
    private const string UninstallKey = @"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\AuthorityGate ShellColors";

    [STAThread]
    private static int Main(string[] args)
    {
        bool silent = HasArgument(args, "/silent") || HasArgument(args, "/quiet") || HasArgument(args, "/s");
        bool uninstall = HasArgument(args, "/uninstall");
        try
        {
            if (!IsAdministrator())
                return RelaunchElevated(uninstall ? (silent ? "/uninstall /silent" : "/uninstall") : (silent ? "/silent" : ""));

            if (uninstall) { Uninstall(silent); return 0; }
            if (silent) { Install(); return 0; }

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new SetupWizard());
            return 0;
        }
        catch (Exception error)
        {
            if (!silent) MessageBox.Show("Setup could not complete.\r\n\r\n" + error.Message, ProductName + " Setup", MessageBoxButtons.OK, MessageBoxIcon.Error);
            return 1;
        }
    }

    private static bool HasArgument(string[] args, string expected)
    {
        return Array.Exists(args, argument => argument.Equals(expected, StringComparison.OrdinalIgnoreCase));
    }

    private static bool IsAdministrator()
    {
        return new WindowsPrincipal(WindowsIdentity.GetCurrent()).IsInRole(WindowsBuiltInRole.Administrator);
    }

    private static int RelaunchElevated(string arguments)
    {
        Process child = Process.Start(new ProcessStartInfo(Process.GetCurrentProcess().MainModule.FileName, arguments)
        {
            Verb = "runas",
            UseShellExecute = true,
            WorkingDirectory = Environment.CurrentDirectory
        });
        if (child == null) return 1;
        child.WaitForExit();
        return child.ExitCode;
    }

    internal static void Install()
    {
        Directory.CreateDirectory(InstallDirectory);
        string application = Path.Combine(InstallDirectory, "ShellColors.exe");
        string icon = Path.Combine(InstallDirectory, "Authority_Gate_CMD.ico");
        string setup = Path.Combine(InstallDirectory, "ShellColorsSetup.exe");
        string registration = Path.Combine(InstallDirectory, "ShellColorsRegistration.exe");
        Extract("ShellColorsApplication", application);
        Extract("ShellColorsIcon", icon);
        Extract("ShellColorsRegistration", registration);
        File.Copy(Process.GetCurrentProcess().MainModule.FileName, setup, true);

        RemoveWindowsIntegration();
        InstallWindowsIntegration(application, icon, setup);

        using (RegistryKey key = Registry.LocalMachine.CreateSubKey(UninstallKey))
        {
            key.SetValue("DisplayName", ProductName);
            key.SetValue("DisplayVersion", Version);
            key.SetValue("Publisher", "AuthorityGate Inc.");
            key.SetValue("InstallLocation", InstallDirectory);
            key.SetValue("DisplayIcon", application);
            key.SetValue("UninstallString", Quote(setup) + " /uninstall");
            key.SetValue("QuietUninstallString", Quote(setup) + " /uninstall /silent");
            key.SetValue("NoModify", 1, RegistryValueKind.DWord);
            key.SetValue("NoRepair", 1, RegistryValueKind.DWord);
            key.SetValue("EstimatedSize", (int)(DirectorySize(InstallDirectory) / 1024), RegistryValueKind.DWord);
        }
        Registry.LocalMachine.DeleteSubKeyTree(@"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\AuthorityGate CMD Help", false);
    }

    private static void InstallWindowsIntegration(string application, string icon, string setup)
    {
        string[,] entries = {
            {@"Directory\shell\AuthorityGateShellColorsAdmin", "AuthorityGate ShellColors (Admin)", Quote(application) + " \"%1\" --admin"},
            {@"Directory\shell\AuthorityGateShellColorsUser", "AuthorityGate ShellColors (User)", Quote(application) + " \"%1\" --user"},
            {@"Directory\shell\AuthorityGateShellColorsSystem", "AuthorityGate ShellColors (System)", Quote(application) + " \"%1\" --system"},
            {@"Directory\shell\AuthorityGateShellColorsPowerShell", "AuthorityGate ShellColors (PowerShell)", Quote(application) + " \"%1\" --powershell"},
            {@"Directory\Background\shell\AuthorityGateShellColorsAdmin", "AuthorityGate ShellColors (Admin)", Quote(application) + " \"%V\" --admin"},
            {@"Directory\Background\shell\AuthorityGateShellColorsUser", "AuthorityGate ShellColors (User)", Quote(application) + " \"%V\" --user"},
            {@"Directory\Background\shell\AuthorityGateShellColorsSystem", "AuthorityGate ShellColors (System)", Quote(application) + " \"%V\" --system"},
            {@"Directory\Background\shell\AuthorityGateShellColorsPowerShell", "AuthorityGate ShellColors (PowerShell)", Quote(application) + " \"%V\" --powershell"}
        };
        for (int index = 0; index < entries.GetLength(0); index++)
        {
            using (RegistryKey key = Registry.ClassesRoot.CreateSubKey(entries[index, 0]))
            {
                key.SetValue(null, entries[index, 1]);
                key.SetValue("Icon", icon);
                using (RegistryKey command = key.CreateSubKey("command")) command.SetValue(null, entries[index, 2]);
            }
        }

        using (RegistryKey key = Registry.LocalMachine.CreateSubKey(@"SOFTWARE\AuthorityGate\ShellColors"))
        {
            SetDefault(key, "AdminTextColor", "4"); SetDefault(key, "UserTextColor", "A"); SetDefault(key, "SystemTextColor", "E");
            SetDefault(key, "AdminBackgroundColor", "0"); SetDefault(key, "UserBackgroundColor", "0"); SetDefault(key, "SystemBackgroundColor", "0");
            key.SetValue("DefaultLocation", InstallDirectory); key.SetValue("IconPath", icon); key.SetValue("Version", Version);
        }

        string menu = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.CommonPrograms), "AuthorityGate Utilities");
        Directory.CreateDirectory(menu);
        CreateShortcut(Path.Combine(menu, "ShellColors (Admin).lnk"), application, "--admin", "Open a red elevated console");
        CreateShortcut(Path.Combine(menu, "ShellColors (User).lnk"), application, "--user", "Open a green user console");
        CreateShortcut(Path.Combine(menu, "ShellColors (System).lnk"), application, "--system", "Open a yellow system console");
        CreateShortcut(Path.Combine(menu, "ShellColors (PowerShell).lnk"), application, "--powershell", "Open a cyan PowerShell console");
        CreateShortcut(Path.Combine(menu, "Uninstall ShellColors.lnk"), setup, "/uninstall", "Uninstall AuthorityGate ShellColors");
    }

    private static void SetDefault(RegistryKey key, string name, string value)
    {
        if (key.GetValue(name) == null) key.SetValue(name, value);
    }

    private static void CreateShortcut(string shortcutPath, string target, string arguments, string description)
    {
        Type type = Type.GetTypeFromProgID("WScript.Shell");
        object shell = Activator.CreateInstance(type);
        object shortcut = type.InvokeMember("CreateShortcut", BindingFlags.InvokeMethod, null, shell, new object[] { shortcutPath });
        Type shortcutType = shortcut.GetType();
        shortcutType.InvokeMember("TargetPath", BindingFlags.SetProperty, null, shortcut, new object[] { target });
        shortcutType.InvokeMember("Arguments", BindingFlags.SetProperty, null, shortcut, new object[] { arguments });
        shortcutType.InvokeMember("WorkingDirectory", BindingFlags.SetProperty, null, shortcut, new object[] { InstallDirectory });
        shortcutType.InvokeMember("Description", BindingFlags.SetProperty, null, shortcut, new object[] { description });
        shortcutType.InvokeMember("Save", BindingFlags.InvokeMethod, null, shortcut, null);
    }

    private static void RemoveWindowsIntegration()
    {
        string[] keys = { "OpenCmdHereAsAdmin", "OpenCmdHereAsUser", "OpenCmdHereAsSystem", "OpenPowerShellHereAuthorityGate",
            "AuthorityGateShellColorsAdmin", "AuthorityGateShellColorsUser", "AuthorityGateShellColorsSystem", "AuthorityGateShellColorsPowerShell" };
        foreach (string key in keys)
        {
            Registry.ClassesRoot.DeleteSubKeyTree(@"Directory\shell\" + key, false);
            Registry.ClassesRoot.DeleteSubKeyTree(@"Directory\Background\shell\" + key, false);
        }
    }

    private static long DirectorySize(string path)
    {
        long total = 0;
        foreach (string file in Directory.GetFiles(path, "*", SearchOption.AllDirectories)) total += new FileInfo(file).Length;
        return total;
    }

    private static void Extract(string resourceName, string destination)
    {
        using (Stream source = Assembly.GetExecutingAssembly().GetManifestResourceStream(resourceName))
        {
            if (source == null) throw new InvalidOperationException("Missing setup resource: " + resourceName);
            using (FileStream target = File.Create(destination)) source.CopyTo(target);
        }
    }

    private static string Quote(string value) { return "\"" + value + "\""; }

    private static void Uninstall(bool silent)
    {
        if (!silent && MessageBox.Show("Remove AuthorityGate ShellColors and its console shortcuts?", ProductName + " Setup", MessageBoxButtons.YesNo, MessageBoxIcon.Question) != DialogResult.Yes) return;
        RemoveWindowsIntegration();
        Registry.LocalMachine.DeleteSubKeyTree(@"SOFTWARE\AuthorityGate\ShellColors", false);
        Registry.LocalMachine.DeleteSubKeyTree(@"SOFTWARE\AuthorityGate\CMDHelper", false);
        Registry.LocalMachine.DeleteSubKeyTree(UninstallKey, false);
        Registry.LocalMachine.DeleteSubKeyTree(@"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\AuthorityGate CMD Help", false);
        string menu = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.CommonPrograms), "AuthorityGate Utilities");
        if (Directory.Exists(menu)) foreach (string file in Directory.GetFiles(menu, "*ShellColors*.lnk")) File.Delete(file);
        string directory = InstallDirectory;
        Process.Start(new ProcessStartInfo("cmd.exe", "/c ping 127.0.0.1 -n 3 > nul & rmdir /s /q " + Quote(directory)) { CreateNoWindow = true, UseShellExecute = false });
        if (!silent) MessageBox.Show("AuthorityGate ShellColors was removed.", ProductName + " Setup", MessageBoxButtons.OK, MessageBoxIcon.Information);
    }
}

internal sealed class SetupWizard : Form
{
    private readonly Label heading = new Label(); private readonly Label body = new Label(); private readonly Label step = new Label();
    private readonly Button back = new Button(); private readonly Button next = new Button(); private readonly Button cancel = new Button();
    private readonly ProgressBar progress = new ProgressBar(); private readonly CheckBox launch = new CheckBox(); private int page;

    internal SetupWizard()
    {
        Text = Program.ProductName + " Setup"; ClientSize = new Size(640, 430); FormBorderStyle = FormBorderStyle.FixedDialog;
        MaximizeBox = false; MinimizeBox = false; StartPosition = FormStartPosition.CenterScreen; BackColor = Color.White; Font = new Font("Segoe UI", 9F);
        Panel banner = new Panel { Dock = DockStyle.Top, Height = 92, BackColor = Color.FromArgb(105, 28, 38) };
        heading.SetBounds(30, 22, 570, 34); heading.Font = new Font("Segoe UI Semibold", 19F); heading.ForeColor = Color.White;
        step.SetBounds(32, 60, 560, 20); step.ForeColor = Color.FromArgb(242, 211, 141); banner.Controls.Add(heading); banner.Controls.Add(step); Controls.Add(banner);
        body.SetBounds(38, 125, 560, 180); body.Font = new Font("Segoe UI", 10F); body.AutoSize = false;
        progress.SetBounds(38, 285, 560, 22); progress.Style = ProgressBarStyle.Marquee; progress.Visible = false;
        launch.SetBounds(38, 286, 560, 25); launch.Text = "Launch ShellColors after setup"; launch.Checked = true; launch.Visible = false;
        Controls.Add(body); Controls.Add(progress); Controls.Add(launch);
        Panel footer = new Panel { Dock = DockStyle.Bottom, Height = 72, BackColor = Color.FromArgb(247, 247, 247) };
        back.SetBounds(330, 19, 88, 34); back.Text = "< Back"; back.Click += delegate { if (page > 0) { page--; Render(); } };
        next.SetBounds(426, 19, 88, 34); next.Text = "Next >"; next.Click += Next;
        cancel.SetBounds(522, 19, 88, 34); cancel.Text = "Cancel"; cancel.Click += delegate { Close(); };
        footer.Controls.Add(back); footer.Controls.Add(next); footer.Controls.Add(cancel); Controls.Add(footer); AcceptButton = next; CancelButton = cancel; Render();
    }

    private void Render()
    {
        back.Enabled = page > 0 && page < 3; progress.Visible = false; launch.Visible = false;
        if (page == 0) { heading.Text = "Welcome to ShellColors Setup"; step.Text = Program.ProductName + " " + Program.Version; body.Text = "Windows 11 removed the practical ability to keep separate colors for different command-line entries.\r\n\r\nShellColors adds that feature back for PowerShell, Administrator, User, and System console windows.\r\n\r\nRegistration is free. Click Next to continue."; next.Text = "Next >"; }
        else if (page == 1) { heading.Text = "Ready to install"; step.Text = "Installation folder"; body.Text = "Setup will install or upgrade ShellColors in:\r\n\r\n" + Program.InstallDirectory + "\r\n\r\nExplorer context-menu entries, Start Menu shortcuts, and Windows uninstall information will be configured."; next.Text = "Install"; }
        else { heading.Text = "Setup complete"; step.Text = "ShellColors " + Program.Version + " is installed"; body.Text = "The newest ShellColors version was installed successfully.\r\n\r\nRegistration opens on first use. Use the AuthorityGate Utilities Start Menu folder or right-click a folder to open a color-coded console."; launch.Visible = true; next.Text = "Finish"; cancel.Enabled = false; }
    }

    private async void Next(object sender, EventArgs e)
    {
        if (page == 0) { page = 1; Render(); return; }
        if (page == 1)
        {
            page = 3; heading.Text = "Installing ShellColors"; step.Text = "Please wait"; body.Text = "Installing the signed application and configuring Windows integration…";
            progress.Visible = true; back.Enabled = false; next.Enabled = false; cancel.Enabled = false;
            try { await System.Threading.Tasks.Task.Run((Action)Program.Install); page = 2; next.Enabled = true; cancel.Enabled = true; Render(); }
            catch (Exception error) { MessageBox.Show("Setup could not complete.\r\n\r\n" + error.Message, Program.ProductName + " Setup", MessageBoxButtons.OK, MessageBoxIcon.Error); page = 1; next.Enabled = true; cancel.Enabled = true; Render(); }
            return;
        }
        if (page == 2)
        {
            if (launch.Checked) Process.Start(new ProcessStartInfo(Path.Combine(Program.InstallDirectory, "ShellColors.exe"), "--user") { UseShellExecute = true, WorkingDirectory = Program.InstallDirectory });
            Close();
        }
    }
}
