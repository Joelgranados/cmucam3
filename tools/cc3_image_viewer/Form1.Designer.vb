<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class Form1
    Inherits System.Windows.Forms.Form

    'Form overrides dispose to clean up the component list.
    <System.Diagnostics.DebuggerNonUserCode()> _
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        If disposing AndAlso components IsNot Nothing Then
            components.Dispose()
        End If
        MyBase.Dispose(disposing)
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()> _
    Private Sub InitializeComponent()
        Me.components = New System.ComponentModel.Container
        Me.Timer1 = New System.Windows.Forms.Timer(Me.components)
        Me.TabControl1 = New System.Windows.Forms.TabControl
        Me.TabPage1 = New System.Windows.Forms.TabPage
        Me.Panel3 = New System.Windows.Forms.Panel
        Me.CIFradio = New System.Windows.Forms.RadioButton
        Me.QCIFradio = New System.Windows.Forms.RadioButton
        Me.Label5 = New System.Windows.Forms.Label
        Me.serialTOBox = New System.Windows.Forms.TextBox
        Me.Label4 = New System.Windows.Forms.Label
        Me.baudBox = New System.Windows.Forms.ComboBox
        Me.Label3 = New System.Windows.Forms.Label
        Me.Label2 = New System.Windows.Forms.Label
        Me.timerTimeout = New System.Windows.Forms.TextBox
        Me.stop_box1 = New System.Windows.Forms.CheckBox
        Me.stopButton = New System.Windows.Forms.Button
        Me.grabButton = New System.Windows.Forms.Button
        Me.PictureBox1 = New System.Windows.Forms.PictureBox
        Me.txtDataReceived = New System.Windows.Forms.RichTextBox
        Me.btnDisconnect = New System.Windows.Forms.Button
        Me.btnConnect = New System.Windows.Forms.Button
        Me.lblMessage = New System.Windows.Forms.Label
        Me.btnSend = New System.Windows.Forms.Button
        Me.txtDataToSend = New System.Windows.Forms.TextBox
        Me.cbbCOMPorts = New System.Windows.Forms.ComboBox
        Me.Label1 = New System.Windows.Forms.Label
        Me.TabPage2 = New System.Windows.Forms.TabPage
        Me.ServoPanel = New System.Windows.Forms.Panel
        Me.TrackBar4 = New System.Windows.Forms.TrackBar
        Me.TrackBar3 = New System.Windows.Forms.TrackBar
        Me.TrackBar2 = New System.Windows.Forms.TrackBar
        Me.Label18 = New System.Windows.Forms.Label
        Me.servo3_textbox = New System.Windows.Forms.TextBox
        Me.servo2_textbox = New System.Windows.Forms.TextBox
        Me.servo1_textbox = New System.Windows.Forms.TextBox
        Me.servo0_textbox = New System.Windows.Forms.TextBox
        Me.Label17 = New System.Windows.Forms.Label
        Me.Label16 = New System.Windows.Forms.Label
        Me.Label15 = New System.Windows.Forms.Label
        Me.Label14 = New System.Windows.Forms.Label
        Me.TrackBar1 = New System.Windows.Forms.TrackBar
        Me.configPanel = New System.Windows.Forms.Panel
        Me.Label13 = New System.Windows.Forms.Label
        Me.Label12 = New System.Windows.Forms.Label
        Me.Label11 = New System.Windows.Forms.Label
        Me.Label10 = New System.Windows.Forms.Label
        Me.Y1_textbox = New System.Windows.Forms.MaskedTextBox
        Me.X1_textbox = New System.Windows.Forms.MaskedTextBox
        Me.Y0_textbox = New System.Windows.Forms.MaskedTextBox
        Me.X0_textbox = New System.Windows.Forms.MaskedTextBox
        Me.Label9 = New System.Windows.Forms.Label
        Me.vwSet = New System.Windows.Forms.Button
        Me.dsSetButton = New System.Windows.Forms.Button
        Me.Label8 = New System.Windows.Forms.Label
        Me.Label7 = New System.Windows.Forms.Label
        Me.dsY = New System.Windows.Forms.NumericUpDown
        Me.Label6 = New System.Windows.Forms.Label
        Me.dsX = New System.Windows.Forms.NumericUpDown
        Me.Panel1 = New System.Windows.Forms.Panel
        Me.loResRadio = New System.Windows.Forms.RadioButton
        Me.HiResRadio = New System.Windows.Forms.RadioButton
        Me.Panel2 = New System.Windows.Forms.Panel
        Me.RGBRadio = New System.Windows.Forms.RadioButton
        Me.YCrCbButton = New System.Windows.Forms.RadioButton
        Me.TabControl1.SuspendLayout()
        Me.TabPage1.SuspendLayout()
        Me.Panel3.SuspendLayout()
        CType(Me.PictureBox1, System.ComponentModel.ISupportInitialize).BeginInit()
        Me.TabPage2.SuspendLayout()
        Me.ServoPanel.SuspendLayout()
        CType(Me.TrackBar4, System.ComponentModel.ISupportInitialize).BeginInit()
        CType(Me.TrackBar3, System.ComponentModel.ISupportInitialize).BeginInit()
        CType(Me.TrackBar2, System.ComponentModel.ISupportInitialize).BeginInit()
        CType(Me.TrackBar1, System.ComponentModel.ISupportInitialize).BeginInit()
        Me.configPanel.SuspendLayout()
        CType(Me.dsY, System.ComponentModel.ISupportInitialize).BeginInit()
        CType(Me.dsX, System.ComponentModel.ISupportInitialize).BeginInit()
        Me.Panel1.SuspendLayout()
        Me.Panel2.SuspendLayout()
        Me.SuspendLayout()
        '
        'Timer1
        '
        Me.Timer1.Interval = 200
        '
        'TabControl1
        '
        Me.TabControl1.Controls.Add(Me.TabPage1)
        Me.TabControl1.Controls.Add(Me.TabPage2)
        Me.TabControl1.Location = New System.Drawing.Point(0, 0)
        Me.TabControl1.Name = "TabControl1"
        Me.TabControl1.SelectedIndex = 0
        Me.TabControl1.Size = New System.Drawing.Size(772, 404)
        Me.TabControl1.TabIndex = 0
        '
        'TabPage1
        '
        Me.TabPage1.Controls.Add(Me.Panel3)
        Me.TabPage1.Controls.Add(Me.Label5)
        Me.TabPage1.Controls.Add(Me.serialTOBox)
        Me.TabPage1.Controls.Add(Me.Label4)
        Me.TabPage1.Controls.Add(Me.baudBox)
        Me.TabPage1.Controls.Add(Me.Label3)
        Me.TabPage1.Controls.Add(Me.Label2)
        Me.TabPage1.Controls.Add(Me.timerTimeout)
        Me.TabPage1.Controls.Add(Me.stop_box1)
        Me.TabPage1.Controls.Add(Me.stopButton)
        Me.TabPage1.Controls.Add(Me.grabButton)
        Me.TabPage1.Controls.Add(Me.PictureBox1)
        Me.TabPage1.Controls.Add(Me.txtDataReceived)
        Me.TabPage1.Controls.Add(Me.btnDisconnect)
        Me.TabPage1.Controls.Add(Me.btnConnect)
        Me.TabPage1.Controls.Add(Me.lblMessage)
        Me.TabPage1.Controls.Add(Me.btnSend)
        Me.TabPage1.Controls.Add(Me.txtDataToSend)
        Me.TabPage1.Controls.Add(Me.cbbCOMPorts)
        Me.TabPage1.Controls.Add(Me.Label1)
        Me.TabPage1.Location = New System.Drawing.Point(4, 22)
        Me.TabPage1.Name = "TabPage1"
        Me.TabPage1.Padding = New System.Windows.Forms.Padding(3)
        Me.TabPage1.Size = New System.Drawing.Size(764, 378)
        Me.TabPage1.TabIndex = 0
        Me.TabPage1.Text = "Main"
        Me.TabPage1.UseVisualStyleBackColor = True
        '
        'Panel3
        '
        Me.Panel3.Controls.Add(Me.CIFradio)
        Me.Panel3.Controls.Add(Me.QCIFradio)
        Me.Panel3.Location = New System.Drawing.Point(408, 308)
        Me.Panel3.Name = "Panel3"
        Me.Panel3.Size = New System.Drawing.Size(103, 24)
        Me.Panel3.TabIndex = 45
        '
        'CIFradio
        '
        Me.CIFradio.AutoSize = True
        Me.CIFradio.Checked = True
        Me.CIFradio.Location = New System.Drawing.Point(3, 2)
        Me.CIFradio.Name = "CIFradio"
        Me.CIFradio.Size = New System.Drawing.Size(41, 17)
        Me.CIFradio.TabIndex = 43
        Me.CIFradio.TabStop = True
        Me.CIFradio.Text = "CIF"
        Me.CIFradio.UseVisualStyleBackColor = True
        '
        'QCIFradio
        '
        Me.QCIFradio.AutoSize = True
        Me.QCIFradio.Location = New System.Drawing.Point(50, 2)
        Me.QCIFradio.Name = "QCIFradio"
        Me.QCIFradio.Size = New System.Drawing.Size(49, 17)
        Me.QCIFradio.TabIndex = 44
        Me.QCIFradio.Text = "QCIF"
        Me.QCIFradio.UseVisualStyleBackColor = True
        '
        'Label5
        '
        Me.Label5.AutoSize = True
        Me.Label5.Location = New System.Drawing.Point(210, 9)
        Me.Label5.Name = "Label5"
        Me.Label5.Size = New System.Drawing.Size(67, 13)
        Me.Label5.TabIndex = 42
        Me.Label5.Text = "Timeout (ms)"
        '
        'serialTOBox
        '
        Me.serialTOBox.Location = New System.Drawing.Point(222, 25)
        Me.serialTOBox.Name = "serialTOBox"
        Me.serialTOBox.Size = New System.Drawing.Size(45, 20)
        Me.serialTOBox.TabIndex = 41
        Me.serialTOBox.Text = "5000"
        '
        'Label4
        '
        Me.Label4.AutoSize = True
        Me.Label4.Location = New System.Drawing.Point(137, 9)
        Me.Label4.Name = "Label4"
        Me.Label4.Size = New System.Drawing.Size(58, 13)
        Me.Label4.TabIndex = 40
        Me.Label4.Text = "Baud Rate"
        '
        'baudBox
        '
        Me.baudBox.FormattingEnabled = True
        Me.baudBox.Location = New System.Drawing.Point(127, 25)
        Me.baudBox.Name = "baudBox"
        Me.baudBox.Size = New System.Drawing.Size(80, 21)
        Me.baudBox.TabIndex = 39
        '
        'Label3
        '
        Me.Label3.AutoSize = True
        Me.Label3.Font = New System.Drawing.Font("Arial", 11.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label3.Location = New System.Drawing.Point(113, 60)
        Me.Label3.Name = "Label3"
        Me.Label3.Size = New System.Drawing.Size(133, 18)
        Me.Label3.TabIndex = 36
        Me.Label3.Text = "Terminal Console"
        '
        'Label2
        '
        Me.Label2.AutoSize = True
        Me.Label2.Location = New System.Drawing.Point(440, 344)
        Me.Label2.Name = "Label2"
        Me.Label2.Size = New System.Drawing.Size(104, 13)
        Me.Label2.TabIndex = 35
        Me.Label2.Text = "Refresh Interval (ms)"
        '
        'timerTimeout
        '
        Me.timerTimeout.Location = New System.Drawing.Point(389, 341)
        Me.timerTimeout.Name = "timerTimeout"
        Me.timerTimeout.Size = New System.Drawing.Size(45, 20)
        Me.timerTimeout.TabIndex = 34
        Me.timerTimeout.Text = "1000"
        '
        'stop_box1
        '
        Me.stop_box1.AutoSize = True
        Me.stop_box1.Location = New System.Drawing.Point(606, 319)
        Me.stop_box1.Name = "stop_box1"
        Me.stop_box1.Size = New System.Drawing.Size(50, 17)
        Me.stop_box1.TabIndex = 33
        Me.stop_box1.Text = "Loop"
        Me.stop_box1.UseVisualStyleBackColor = True
        '
        'stopButton
        '
        Me.stopButton.Enabled = False
        Me.stopButton.Location = New System.Drawing.Point(662, 344)
        Me.stopButton.Name = "stopButton"
        Me.stopButton.Size = New System.Drawing.Size(75, 23)
        Me.stopButton.TabIndex = 32
        Me.stopButton.Text = "Stop"
        Me.stopButton.UseVisualStyleBackColor = True
        '
        'grabButton
        '
        Me.grabButton.Enabled = False
        Me.grabButton.Location = New System.Drawing.Point(662, 315)
        Me.grabButton.Name = "grabButton"
        Me.grabButton.Size = New System.Drawing.Size(75, 23)
        Me.grabButton.TabIndex = 31
        Me.grabButton.Text = "Grab Image"
        Me.grabButton.UseVisualStyleBackColor = True
        '
        'PictureBox1
        '
        Me.PictureBox1.BackColor = System.Drawing.Color.Black
        Me.PictureBox1.BackgroundImageLayout = System.Windows.Forms.ImageLayout.None
        Me.PictureBox1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.PictureBox1.Cursor = System.Windows.Forms.Cursors.Cross
        Me.PictureBox1.InitialImage = Nothing
        Me.PictureBox1.Location = New System.Drawing.Point(385, 12)
        Me.PictureBox1.Name = "PictureBox1"
        Me.PictureBox1.Size = New System.Drawing.Size(352, 288)
        Me.PictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage
        Me.PictureBox1.TabIndex = 30
        Me.PictureBox1.TabStop = False
        '
        'txtDataReceived
        '
        Me.txtDataReceived.Font = New System.Drawing.Font("Courier New", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.txtDataReceived.Location = New System.Drawing.Point(6, 81)
        Me.txtDataReceived.Name = "txtDataReceived"
        Me.txtDataReceived.ScrollBars = System.Windows.Forms.RichTextBoxScrollBars.Vertical
        Me.txtDataReceived.Size = New System.Drawing.Size(352, 219)
        Me.txtDataReceived.TabIndex = 29
        Me.txtDataReceived.Text = ""
        '
        'btnDisconnect
        '
        Me.btnDisconnect.Location = New System.Drawing.Point(283, 31)
        Me.btnDisconnect.Name = "btnDisconnect"
        Me.btnDisconnect.Size = New System.Drawing.Size(75, 23)
        Me.btnDisconnect.TabIndex = 28
        Me.btnDisconnect.Text = "Disconnect"
        Me.btnDisconnect.UseVisualStyleBackColor = True
        '
        'btnConnect
        '
        Me.btnConnect.Location = New System.Drawing.Point(283, 4)
        Me.btnConnect.Name = "btnConnect"
        Me.btnConnect.Size = New System.Drawing.Size(75, 23)
        Me.btnConnect.TabIndex = 27
        Me.btnConnect.Text = "Connect"
        Me.btnConnect.UseVisualStyleBackColor = True
        '
        'lblMessage
        '
        Me.lblMessage.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle
        Me.lblMessage.Location = New System.Drawing.Point(6, 339)
        Me.lblMessage.Name = "lblMessage"
        Me.lblMessage.Size = New System.Drawing.Size(352, 23)
        Me.lblMessage.TabIndex = 26
        '
        'btnSend
        '
        Me.btnSend.Enabled = False
        Me.btnSend.Location = New System.Drawing.Point(283, 306)
        Me.btnSend.Name = "btnSend"
        Me.btnSend.Size = New System.Drawing.Size(75, 23)
        Me.btnSend.TabIndex = 0
        Me.btnSend.Text = "Send"
        Me.btnSend.UseVisualStyleBackColor = True
        '
        'txtDataToSend
        '
        Me.txtDataToSend.Location = New System.Drawing.Point(4, 308)
        Me.txtDataToSend.Multiline = True
        Me.txtDataToSend.Name = "txtDataToSend"
        Me.txtDataToSend.Size = New System.Drawing.Size(273, 20)
        Me.txtDataToSend.TabIndex = 24
        '
        'cbbCOMPorts
        '
        Me.cbbCOMPorts.FormattingEnabled = True
        Me.cbbCOMPorts.Location = New System.Drawing.Point(11, 25)
        Me.cbbCOMPorts.Name = "cbbCOMPorts"
        Me.cbbCOMPorts.Size = New System.Drawing.Size(101, 21)
        Me.cbbCOMPorts.TabIndex = 23
        '
        'Label1
        '
        Me.Label1.AutoSize = True
        Me.Label1.Location = New System.Drawing.Point(8, 9)
        Me.Label1.Name = "Label1"
        Me.Label1.Size = New System.Drawing.Size(104, 13)
        Me.Label1.TabIndex = 22
        Me.Label1.Text = "Available COM Ports"
        '
        'TabPage2
        '
        Me.TabPage2.Controls.Add(Me.ServoPanel)
        Me.TabPage2.Controls.Add(Me.configPanel)
        Me.TabPage2.Location = New System.Drawing.Point(4, 22)
        Me.TabPage2.Name = "TabPage2"
        Me.TabPage2.Padding = New System.Windows.Forms.Padding(3)
        Me.TabPage2.Size = New System.Drawing.Size(764, 378)
        Me.TabPage2.TabIndex = 1
        Me.TabPage2.Text = "Settings"
        Me.TabPage2.UseVisualStyleBackColor = True
        '
        'ServoPanel
        '
        Me.ServoPanel.Controls.Add(Me.TrackBar4)
        Me.ServoPanel.Controls.Add(Me.TrackBar3)
        Me.ServoPanel.Controls.Add(Me.TrackBar2)
        Me.ServoPanel.Controls.Add(Me.Label18)
        Me.ServoPanel.Controls.Add(Me.servo3_textbox)
        Me.ServoPanel.Controls.Add(Me.servo2_textbox)
        Me.ServoPanel.Controls.Add(Me.servo1_textbox)
        Me.ServoPanel.Controls.Add(Me.servo0_textbox)
        Me.ServoPanel.Controls.Add(Me.Label17)
        Me.ServoPanel.Controls.Add(Me.Label16)
        Me.ServoPanel.Controls.Add(Me.Label15)
        Me.ServoPanel.Controls.Add(Me.Label14)
        Me.ServoPanel.Controls.Add(Me.TrackBar1)
        Me.ServoPanel.Enabled = False
        Me.ServoPanel.Location = New System.Drawing.Point(478, 133)
        Me.ServoPanel.Name = "ServoPanel"
        Me.ServoPanel.Size = New System.Drawing.Size(277, 200)
        Me.ServoPanel.TabIndex = 7
        '
        'TrackBar4
        '
        Me.TrackBar4.Location = New System.Drawing.Point(28, 156)
        Me.TrackBar4.Maximum = 255
        Me.TrackBar4.Name = "TrackBar4"
        Me.TrackBar4.Size = New System.Drawing.Size(170, 42)
        Me.TrackBar4.TabIndex = 22
        Me.TrackBar4.TickFrequency = 8
        Me.TrackBar4.Value = 128
        '
        'TrackBar3
        '
        Me.TrackBar3.Location = New System.Drawing.Point(28, 117)
        Me.TrackBar3.Maximum = 255
        Me.TrackBar3.Name = "TrackBar3"
        Me.TrackBar3.Size = New System.Drawing.Size(170, 42)
        Me.TrackBar3.TabIndex = 21
        Me.TrackBar3.TickFrequency = 8
        Me.TrackBar3.Value = 128
        '
        'TrackBar2
        '
        Me.TrackBar2.Location = New System.Drawing.Point(28, 79)
        Me.TrackBar2.Maximum = 255
        Me.TrackBar2.Name = "TrackBar2"
        Me.TrackBar2.Size = New System.Drawing.Size(170, 42)
        Me.TrackBar2.TabIndex = 20
        Me.TrackBar2.TickFrequency = 8
        Me.TrackBar2.Value = 128
        '
        'Label18
        '
        Me.Label18.AutoSize = True
        Me.Label18.Location = New System.Drawing.Point(68, 11)
        Me.Label18.Name = "Label18"
        Me.Label18.Size = New System.Drawing.Size(82, 13)
        Me.Label18.TabIndex = 19
        Me.Label18.Text = "Servo Controller"
        '
        'servo3_textbox
        '
        Me.servo3_textbox.Location = New System.Drawing.Point(213, 156)
        Me.servo3_textbox.Name = "servo3_textbox"
        Me.servo3_textbox.Size = New System.Drawing.Size(42, 20)
        Me.servo3_textbox.TabIndex = 18
        Me.servo3_textbox.Text = "128"
        '
        'servo2_textbox
        '
        Me.servo2_textbox.Location = New System.Drawing.Point(213, 117)
        Me.servo2_textbox.Name = "servo2_textbox"
        Me.servo2_textbox.Size = New System.Drawing.Size(42, 20)
        Me.servo2_textbox.TabIndex = 17
        Me.servo2_textbox.Text = "128"
        '
        'servo1_textbox
        '
        Me.servo1_textbox.Location = New System.Drawing.Point(213, 79)
        Me.servo1_textbox.Name = "servo1_textbox"
        Me.servo1_textbox.Size = New System.Drawing.Size(42, 20)
        Me.servo1_textbox.TabIndex = 16
        Me.servo1_textbox.Text = "128"
        '
        'servo0_textbox
        '
        Me.servo0_textbox.Location = New System.Drawing.Point(213, 45)
        Me.servo0_textbox.Name = "servo0_textbox"
        Me.servo0_textbox.Size = New System.Drawing.Size(42, 20)
        Me.servo0_textbox.TabIndex = 15
        Me.servo0_textbox.Text = "128"
        '
        'Label17
        '
        Me.Label17.AutoSize = True
        Me.Label17.Location = New System.Drawing.Point(9, 163)
        Me.Label17.Name = "Label17"
        Me.Label17.Size = New System.Drawing.Size(13, 13)
        Me.Label17.TabIndex = 14
        Me.Label17.Text = "3"
        '
        'Label16
        '
        Me.Label16.AutoSize = True
        Me.Label16.Location = New System.Drawing.Point(9, 124)
        Me.Label16.Name = "Label16"
        Me.Label16.Size = New System.Drawing.Size(13, 13)
        Me.Label16.TabIndex = 12
        Me.Label16.Text = "2"
        '
        'Label15
        '
        Me.Label15.AutoSize = True
        Me.Label15.Location = New System.Drawing.Point(9, 86)
        Me.Label15.Name = "Label15"
        Me.Label15.Size = New System.Drawing.Size(13, 13)
        Me.Label15.TabIndex = 10
        Me.Label15.Text = "1"
        '
        'Label14
        '
        Me.Label14.AutoSize = True
        Me.Label14.Location = New System.Drawing.Point(9, 48)
        Me.Label14.Name = "Label14"
        Me.Label14.Size = New System.Drawing.Size(13, 13)
        Me.Label14.TabIndex = 8
        Me.Label14.Text = "0"
        '
        'TrackBar1
        '
        Me.TrackBar1.Location = New System.Drawing.Point(28, 35)
        Me.TrackBar1.Maximum = 255
        Me.TrackBar1.Name = "TrackBar1"
        Me.TrackBar1.Size = New System.Drawing.Size(170, 42)
        Me.TrackBar1.TabIndex = 7
        Me.TrackBar1.TickFrequency = 8
        Me.TrackBar1.Value = 128
        '
        'configPanel
        '
        Me.configPanel.Controls.Add(Me.Label13)
        Me.configPanel.Controls.Add(Me.Label12)
        Me.configPanel.Controls.Add(Me.Label11)
        Me.configPanel.Controls.Add(Me.Label10)
        Me.configPanel.Controls.Add(Me.Y1_textbox)
        Me.configPanel.Controls.Add(Me.X1_textbox)
        Me.configPanel.Controls.Add(Me.Y0_textbox)
        Me.configPanel.Controls.Add(Me.X0_textbox)
        Me.configPanel.Controls.Add(Me.Label9)
        Me.configPanel.Controls.Add(Me.vwSet)
        Me.configPanel.Controls.Add(Me.dsSetButton)
        Me.configPanel.Controls.Add(Me.Label8)
        Me.configPanel.Controls.Add(Me.Label7)
        Me.configPanel.Controls.Add(Me.dsY)
        Me.configPanel.Controls.Add(Me.Label6)
        Me.configPanel.Controls.Add(Me.dsX)
        Me.configPanel.Controls.Add(Me.Panel1)
        Me.configPanel.Controls.Add(Me.Panel2)
        Me.configPanel.Enabled = False
        Me.configPanel.Location = New System.Drawing.Point(8, 32)
        Me.configPanel.Name = "configPanel"
        Me.configPanel.Size = New System.Drawing.Size(453, 301)
        Me.configPanel.TabIndex = 6
        '
        'Label13
        '
        Me.Label13.AutoSize = True
        Me.Label13.Location = New System.Drawing.Point(302, 151)
        Me.Label13.Name = "Label13"
        Me.Label13.Size = New System.Drawing.Size(20, 13)
        Me.Label13.TabIndex = 20
        Me.Label13.Text = "Y1"
        '
        'Label12
        '
        Me.Label12.AutoSize = True
        Me.Label12.Location = New System.Drawing.Point(227, 151)
        Me.Label12.Name = "Label12"
        Me.Label12.Size = New System.Drawing.Size(20, 13)
        Me.Label12.TabIndex = 19
        Me.Label12.Text = "X1"
        '
        'Label11
        '
        Me.Label11.AutoSize = True
        Me.Label11.Location = New System.Drawing.Point(154, 150)
        Me.Label11.Name = "Label11"
        Me.Label11.Size = New System.Drawing.Size(20, 13)
        Me.Label11.TabIndex = 18
        Me.Label11.Text = "Y0"
        '
        'Label10
        '
        Me.Label10.AutoSize = True
        Me.Label10.Location = New System.Drawing.Point(79, 151)
        Me.Label10.Name = "Label10"
        Me.Label10.Size = New System.Drawing.Size(20, 13)
        Me.Label10.TabIndex = 17
        Me.Label10.Text = "X0"
        '
        'Y1_textbox
        '
        Me.Y1_textbox.Location = New System.Drawing.Point(326, 148)
        Me.Y1_textbox.Name = "Y1_textbox"
        Me.Y1_textbox.Size = New System.Drawing.Size(43, 20)
        Me.Y1_textbox.TabIndex = 16
        Me.Y1_textbox.Text = "144"
        '
        'X1_textbox
        '
        Me.X1_textbox.Location = New System.Drawing.Point(253, 148)
        Me.X1_textbox.Name = "X1_textbox"
        Me.X1_textbox.Size = New System.Drawing.Size(43, 20)
        Me.X1_textbox.TabIndex = 15
        Me.X1_textbox.Text = "176"
        '
        'Y0_textbox
        '
        Me.Y0_textbox.Location = New System.Drawing.Point(174, 147)
        Me.Y0_textbox.Name = "Y0_textbox"
        Me.Y0_textbox.Size = New System.Drawing.Size(43, 20)
        Me.Y0_textbox.TabIndex = 14
        Me.Y0_textbox.Text = "0"
        '
        'X0_textbox
        '
        Me.X0_textbox.Location = New System.Drawing.Point(105, 147)
        Me.X0_textbox.Name = "X0_textbox"
        Me.X0_textbox.Size = New System.Drawing.Size(43, 20)
        Me.X0_textbox.TabIndex = 7
        Me.X0_textbox.Text = "0"
        '
        'Label9
        '
        Me.Label9.AutoSize = True
        Me.Label9.Location = New System.Drawing.Point(11, 150)
        Me.Label9.Name = "Label9"
        Me.Label9.Size = New System.Drawing.Size(46, 13)
        Me.Label9.TabIndex = 13
        Me.Label9.Text = "Window"
        '
        'vwSet
        '
        Me.vwSet.Location = New System.Drawing.Point(386, 148)
        Me.vwSet.Name = "vwSet"
        Me.vwSet.Size = New System.Drawing.Size(47, 20)
        Me.vwSet.TabIndex = 12
        Me.vwSet.Text = "Set"
        Me.vwSet.UseVisualStyleBackColor = True
        '
        'dsSetButton
        '
        Me.dsSetButton.Location = New System.Drawing.Point(253, 110)
        Me.dsSetButton.Name = "dsSetButton"
        Me.dsSetButton.Size = New System.Drawing.Size(47, 20)
        Me.dsSetButton.TabIndex = 11
        Me.dsSetButton.Text = "Set"
        Me.dsSetButton.UseVisualStyleBackColor = True
        '
        'Label8
        '
        Me.Label8.AutoSize = True
        Me.Label8.Location = New System.Drawing.Point(171, 112)
        Me.Label8.Name = "Label8"
        Me.Label8.Size = New System.Drawing.Size(14, 13)
        Me.Label8.TabIndex = 10
        Me.Label8.Text = "Y"
        '
        'Label7
        '
        Me.Label7.AutoSize = True
        Me.Label7.Location = New System.Drawing.Point(102, 112)
        Me.Label7.Name = "Label7"
        Me.Label7.Size = New System.Drawing.Size(14, 13)
        Me.Label7.TabIndex = 9
        Me.Label7.Text = "X"
        '
        'dsY
        '
        Me.dsY.Location = New System.Drawing.Point(191, 110)
        Me.dsY.Maximum = New Decimal(New Integer() {10, 0, 0, 0})
        Me.dsY.Minimum = New Decimal(New Integer() {1, 0, 0, 0})
        Me.dsY.Name = "dsY"
        Me.dsY.Size = New System.Drawing.Size(39, 20)
        Me.dsY.TabIndex = 8
        Me.dsY.Value = New Decimal(New Integer() {1, 0, 0, 0})
        '
        'Label6
        '
        Me.Label6.AutoSize = True
        Me.Label6.Location = New System.Drawing.Point(11, 112)
        Me.Label6.Name = "Label6"
        Me.Label6.Size = New System.Drawing.Size(73, 13)
        Me.Label6.TabIndex = 7
        Me.Label6.Text = "Down Sample"
        '
        'dsX
        '
        Me.dsX.Location = New System.Drawing.Point(121, 110)
        Me.dsX.Maximum = New Decimal(New Integer() {10, 0, 0, 0})
        Me.dsX.Minimum = New Decimal(New Integer() {1, 0, 0, 0})
        Me.dsX.Name = "dsX"
        Me.dsX.Size = New System.Drawing.Size(39, 20)
        Me.dsX.TabIndex = 6
        Me.dsX.Value = New Decimal(New Integer() {1, 0, 0, 0})
        '
        'Panel1
        '
        Me.Panel1.Controls.Add(Me.loResRadio)
        Me.Panel1.Controls.Add(Me.HiResRadio)
        Me.Panel1.Location = New System.Drawing.Point(14, 22)
        Me.Panel1.Name = "Panel1"
        Me.Panel1.Size = New System.Drawing.Size(229, 31)
        Me.Panel1.TabIndex = 4
        '
        'loResRadio
        '
        Me.loResRadio.AutoSize = True
        Me.loResRadio.Checked = True
        Me.loResRadio.Location = New System.Drawing.Point(3, 3)
        Me.loResRadio.Name = "loResRadio"
        Me.loResRadio.Size = New System.Drawing.Size(98, 17)
        Me.loResRadio.TabIndex = 0
        Me.loResRadio.TabStop = True
        Me.loResRadio.Text = "Low Resolution"
        Me.loResRadio.UseVisualStyleBackColor = True
        '
        'HiResRadio
        '
        Me.HiResRadio.AutoSize = True
        Me.HiResRadio.Location = New System.Drawing.Point(107, 3)
        Me.HiResRadio.Name = "HiResRadio"
        Me.HiResRadio.Size = New System.Drawing.Size(100, 17)
        Me.HiResRadio.TabIndex = 1
        Me.HiResRadio.Text = "High Resolution"
        Me.HiResRadio.UseVisualStyleBackColor = True
        '
        'Panel2
        '
        Me.Panel2.Controls.Add(Me.RGBRadio)
        Me.Panel2.Controls.Add(Me.YCrCbButton)
        Me.Panel2.Location = New System.Drawing.Point(14, 59)
        Me.Panel2.Name = "Panel2"
        Me.Panel2.Size = New System.Drawing.Size(229, 31)
        Me.Panel2.TabIndex = 5
        '
        'RGBRadio
        '
        Me.RGBRadio.AutoSize = True
        Me.RGBRadio.Checked = True
        Me.RGBRadio.Location = New System.Drawing.Point(3, 3)
        Me.RGBRadio.Name = "RGBRadio"
        Me.RGBRadio.Size = New System.Drawing.Size(48, 17)
        Me.RGBRadio.TabIndex = 2
        Me.RGBRadio.TabStop = True
        Me.RGBRadio.Text = "RGB"
        Me.RGBRadio.UseVisualStyleBackColor = True
        '
        'YCrCbButton
        '
        Me.YCrCbButton.AutoSize = True
        Me.YCrCbButton.Location = New System.Drawing.Point(104, 3)
        Me.YCrCbButton.Name = "YCrCbButton"
        Me.YCrCbButton.Size = New System.Drawing.Size(55, 17)
        Me.YCrCbButton.TabIndex = 3
        Me.YCrCbButton.Text = "YCrCb"
        Me.YCrCbButton.UseVisualStyleBackColor = True
        '
        'Form1
        '
        Me.AcceptButton = Me.btnSend
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.BackColor = System.Drawing.Color.Silver
        Me.ClientSize = New System.Drawing.Size(771, 404)
        Me.Controls.Add(Me.TabControl1)
        Me.Name = "Form1"
        Me.Text = "CMUcam Frame Grabber"
        Me.TabControl1.ResumeLayout(False)
        Me.TabPage1.ResumeLayout(False)
        Me.TabPage1.PerformLayout()
        Me.Panel3.ResumeLayout(False)
        Me.Panel3.PerformLayout()
        CType(Me.PictureBox1, System.ComponentModel.ISupportInitialize).EndInit()
        Me.TabPage2.ResumeLayout(False)
        Me.ServoPanel.ResumeLayout(False)
        Me.ServoPanel.PerformLayout()
        CType(Me.TrackBar4, System.ComponentModel.ISupportInitialize).EndInit()
        CType(Me.TrackBar3, System.ComponentModel.ISupportInitialize).EndInit()
        CType(Me.TrackBar2, System.ComponentModel.ISupportInitialize).EndInit()
        CType(Me.TrackBar1, System.ComponentModel.ISupportInitialize).EndInit()
        Me.configPanel.ResumeLayout(False)
        Me.configPanel.PerformLayout()
        CType(Me.dsY, System.ComponentModel.ISupportInitialize).EndInit()
        CType(Me.dsX, System.ComponentModel.ISupportInitialize).EndInit()
        Me.Panel1.ResumeLayout(False)
        Me.Panel1.PerformLayout()
        Me.Panel2.ResumeLayout(False)
        Me.Panel2.PerformLayout()
        Me.ResumeLayout(False)

    End Sub
    Friend WithEvents Timer1 As System.Windows.Forms.Timer
    Friend WithEvents TabControl1 As System.Windows.Forms.TabControl
    Friend WithEvents TabPage1 As System.Windows.Forms.TabPage
    Friend WithEvents Label5 As System.Windows.Forms.Label
    Friend WithEvents serialTOBox As System.Windows.Forms.TextBox
    Friend WithEvents Label4 As System.Windows.Forms.Label
    Friend WithEvents baudBox As System.Windows.Forms.ComboBox
    Friend WithEvents Label3 As System.Windows.Forms.Label
    Friend WithEvents Label2 As System.Windows.Forms.Label
    Friend WithEvents timerTimeout As System.Windows.Forms.TextBox
    Friend WithEvents stop_box1 As System.Windows.Forms.CheckBox
    Friend WithEvents stopButton As System.Windows.Forms.Button
    Friend WithEvents grabButton As System.Windows.Forms.Button
    Friend WithEvents PictureBox1 As System.Windows.Forms.PictureBox
    Friend WithEvents txtDataReceived As System.Windows.Forms.RichTextBox
    Friend WithEvents btnDisconnect As System.Windows.Forms.Button
    Friend WithEvents btnConnect As System.Windows.Forms.Button
    Friend WithEvents lblMessage As System.Windows.Forms.Label
    Friend WithEvents btnSend As System.Windows.Forms.Button
    Friend WithEvents txtDataToSend As System.Windows.Forms.TextBox
    Friend WithEvents cbbCOMPorts As System.Windows.Forms.ComboBox
    Friend WithEvents Label1 As System.Windows.Forms.Label
    Friend WithEvents TabPage2 As System.Windows.Forms.TabPage
    Friend WithEvents HiResRadio As System.Windows.Forms.RadioButton
    Friend WithEvents loResRadio As System.Windows.Forms.RadioButton
    Friend WithEvents YCrCbButton As System.Windows.Forms.RadioButton
    Friend WithEvents RGBRadio As System.Windows.Forms.RadioButton
    Friend WithEvents Panel2 As System.Windows.Forms.Panel
    Friend WithEvents Panel1 As System.Windows.Forms.Panel
    Friend WithEvents configPanel As System.Windows.Forms.Panel
    Friend WithEvents CIFradio As System.Windows.Forms.RadioButton
    Friend WithEvents QCIFradio As System.Windows.Forms.RadioButton
    Friend WithEvents Panel3 As System.Windows.Forms.Panel
    Friend WithEvents Label6 As System.Windows.Forms.Label
    Friend WithEvents dsX As System.Windows.Forms.NumericUpDown
    Friend WithEvents Label10 As System.Windows.Forms.Label
    Friend WithEvents Y1_textbox As System.Windows.Forms.MaskedTextBox
    Friend WithEvents X1_textbox As System.Windows.Forms.MaskedTextBox
    Friend WithEvents Y0_textbox As System.Windows.Forms.MaskedTextBox
    Friend WithEvents X0_textbox As System.Windows.Forms.MaskedTextBox
    Friend WithEvents Label9 As System.Windows.Forms.Label
    Friend WithEvents vwSet As System.Windows.Forms.Button
    Friend WithEvents dsSetButton As System.Windows.Forms.Button
    Friend WithEvents Label8 As System.Windows.Forms.Label
    Friend WithEvents Label7 As System.Windows.Forms.Label
    Friend WithEvents dsY As System.Windows.Forms.NumericUpDown
    Friend WithEvents Label13 As System.Windows.Forms.Label
    Friend WithEvents Label12 As System.Windows.Forms.Label
    Friend WithEvents Label11 As System.Windows.Forms.Label
    Friend WithEvents ServoPanel As System.Windows.Forms.Panel
    Friend WithEvents servo1_textbox As System.Windows.Forms.TextBox
    Friend WithEvents servo0_textbox As System.Windows.Forms.TextBox
    Friend WithEvents Label17 As System.Windows.Forms.Label
    Friend WithEvents Label16 As System.Windows.Forms.Label
    Friend WithEvents Label15 As System.Windows.Forms.Label
    Friend WithEvents Label14 As System.Windows.Forms.Label
    Friend WithEvents TrackBar1 As System.Windows.Forms.TrackBar
    Friend WithEvents TrackBar4 As System.Windows.Forms.TrackBar
    Friend WithEvents TrackBar3 As System.Windows.Forms.TrackBar
    Friend WithEvents TrackBar2 As System.Windows.Forms.TrackBar
    Friend WithEvents Label18 As System.Windows.Forms.Label
    Friend WithEvents servo3_textbox As System.Windows.Forms.TextBox
    Friend WithEvents servo2_textbox As System.Windows.Forms.TextBox

End Class
