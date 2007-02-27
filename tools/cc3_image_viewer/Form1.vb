Public Class Form1
    Dim WithEvents serialPort As New IO.Ports.SerialPort
    Public jpeg_grab As Byte
    Public myStr As String






    Private Sub Form1_Load( _
       ByVal sender As System.Object, _
       ByVal e As System.EventArgs) _
       Handles MyBase.Load


        For i As Integer = 0 To _
           My.Computer.Ports.SerialPortNames.Count - 1
            cbbCOMPorts.Items.Add( _
               My.Computer.Ports.SerialPortNames(i))
        Next
        cbbCOMPorts.SelectedIndex = 0
        baudBox.Items.Add("115200")
        baudBox.Items.Add("57600")
        baudBox.Items.Add("38400")
        baudBox.Items.Add("28800")
        baudBox.Items.Add("14400")
        baudBox.Items.Add("9600")
        baudBox.Items.Add("4800")
        baudBox.Items.Add("2400")
        baudBox.Items.Add("1200")
        baudBox.SelectedIndex = 0
        btnDisconnect.Enabled = False
        jpeg_grab = 0
    End Sub

    Private Sub DataReceived( _
       ByVal sender As Object, _
       ByVal e As System.IO.Ports.SerialDataReceivedEventArgs) _
           Handles serialPort.DataReceived

        If (jpeg_grab = 0) Then
            txtDataReceived.Invoke(New _
              myDelegate(AddressOf updateTextBox), _
             New Object() {})
        End If

    End Sub


    Public Delegate Sub myDelegate()
    Public Sub updateTextBox()
        With txtDataReceived
            .Font = New Font("Courier", 9.0!, FontStyle.Bold)
            .SelectionColor = Color.Red
            .AppendText(serialPort.ReadExisting)
            .ScrollToCaret()
        End With
    End Sub

    

    Private Sub StreamStop()
        Try
            Timer1.Stop()
            serialPort.DiscardInBuffer()
            serialPort.DiscardOutBuffer()
            jpeg_grab = 0
        Catch ex As Exception

        End Try
    End Sub



    Private Sub grab_jpeg()
        Try
            Dim i As Integer
            Dim buf() As Byte


            jpeg_grab = 1
            serialPort.DiscardInBuffer()
            serialPort.Write("SJ" & vbCr)

            serialPort.ReadTo("ACK" & vbCr)

            myStr = serialPort.ReadTo("JPG_END" & vbCr & ":")

            ReDim buf(myStr.Length())

            For i = 0 To myStr.Length() - 1
                buf(i) = CByte(Asc(myStr(i)))
            Next

            My.Computer.FileSystem.WriteAllBytes("test.jpg", buf, False)

            jpeg_grab = 0

            PictureBox1.ImageLocation = "test.jpg"
            PictureBox1.Refresh()

        Catch ex As Exception
            MsgBox("Frame Grab Comm. Error")
            StreamStop()
        End Try
    End Sub



    Private Sub Timer1_Tick(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Timer1.Tick
        If jpeg_grab = 1 Then
            StreamStop()
            MsgBox("Refresh Interval Too Short!")
            Return
        End If
        grab_jpeg()

    End Sub


    


    Private Sub btnConnect_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnConnect.Click

        If serialPort.IsOpen Then
            serialPort.Close()
        End If
        Try

            With serialPort
                .PortName = cbbCOMPorts.Text
                .BaudRate = Val(baudBox.SelectedItem().ToString())
                .Parity = IO.Ports.Parity.None
                .DataBits = 8
                .StopBits = IO.Ports.StopBits.One
                .ReadTimeout = Val(serialTOBox.Text())
                '.Encoding = System.Text.Encoding.Unicode
                .Encoding = System.Text.Encoding.Default


            End With
            serialPort.Open()
            serialPort.DiscardInBuffer()
            serialPort.DiscardOutBuffer()

            lblMessage.Text = cbbCOMPorts.Text & " connected."
            btnConnect.Enabled = False
            btnDisconnect.Enabled = True
            grabButton.Enabled = True
            btnSend.Enabled = True
            stopButton.Enabled = True
            configPanel.Enabled = True
            ServoPanel.Enabled = True
            txtDataToSend.Focus()

        Catch ex As Exception
            MsgBox("Communication Error")
            StreamStop()
        End Try
    End Sub

    Private Sub btnDisconnect_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnDisconnect.Click
        Try
            serialPort.Close()
            lblMessage.Text = serialPort.PortName & " disconnected."
            btnConnect.Enabled = True
            btnDisconnect.Enabled = False
            grabButton.Enabled = False
            btnSend.Enabled = False
            stopButton.Enabled = False
            configPanel.Enabled = False
            ServoPanel.Enabled = False
        Catch ex As Exception
            MsgBox("Communication Error")
            StreamStop()
        End Try
    End Sub

    Private Sub grabButton_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles grabButton.Click
        serialPort.ReadTimeout = Val(serialTOBox.Text())
        If stop_box1.Checked() = True Then
            Timer1.Interval = Val(timerTimeout.Text())
            Timer1.Start()
        End If

        grab_jpeg()
    End Sub

    Private Sub stopButton_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles stopButton.Click
        Timer1.Stop()
    End Sub

    
    Private Sub CIFradio_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CIFradio.CheckedChanged
        PictureBox1.Height = 288
        PictureBox1.Width = 352

    End Sub

    Private Sub QCIFradio_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles QCIFradio.CheckedChanged
        PictureBox1.Height = 144
        PictureBox1.Width = 176

    End Sub


  
  
    Private Sub btnSend_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnSend.Click
        Try
            serialPort.Write(txtDataToSend.Text & vbCr)
            With txtDataReceived
                .SelectionColor = Color.Black
                .AppendText(txtDataToSend.Text & vbCr)
                .ScrollToCaret()
            End With
            txtDataToSend.Text = String.Empty
        Catch ex As Exception
            MsgBox("Communication Error")
            StreamStop()
        End Try
    End Sub

  
   
    
    Private Sub vwSet_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles vwSet.Click
        serialPort.Write("VW " & X0_textbox.Text & " " & Y0_textbox.Text & " " & X1_textbox.Text & " " & Y1_textbox.Text & vbCr)
    End Sub

   
    Private Sub HiResRadio_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles HiResRadio.CheckedChanged
        serialPort.Write("HR 1" & vbCr)
    End Sub

    Private Sub loResRadio_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles loResRadio.CheckedChanged
        If (configPanel.Enabled = True) Then
            serialPort.Write("HR 0" & vbCr)
        End If

    End Sub

    Private Sub RGBRadio_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles RGBRadio.CheckedChanged
        If (configPanel.Enabled = True) Then
            serialPort.Write("CR 18 44" & vbCr)
        End If
    End Sub

    Private Sub YCrCbButton_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles YCrCbButton.CheckedChanged
        If (configPanel.Enabled = True) Then
            serialPort.Write("CR 18 36" & vbCr)
        End If

    End Sub

    Private Sub dsSetButton_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles dsSetButton.Click
        If (configPanel.Enabled = True) Then
            serialPort.Write("DS " & dsX.Value & " " & dsY.Value & vbCr)
        End If

    End Sub

    Private Sub TrackBar1_Scroll(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles TrackBar1.Scroll
        servo0_textbox.Clear()
        servo0_textbox.AppendText(TrackBar1.Value)
        serialPort.Write("SV 0 " & servo0_textbox.Text & vbCr)
    End Sub

    Private Sub TrackBar2_Scroll(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles TrackBar2.Scroll
        servo1_textbox.Clear()
        servo1_textbox.AppendText(TrackBar2.Value)
        serialPort.Write("SV 1 " & servo1_textbox.Text & vbCr)
    End Sub

    Private Sub TrackBar3_Scroll(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles TrackBar3.Scroll
        servo2_textbox.Clear()
        servo2_textbox.AppendText(TrackBar3.Value)
        serialPort.Write("SV 2 " & servo2_textbox.Text & vbCr)
    End Sub

    Private Sub TrackBar4_Scroll(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles TrackBar4.Scroll
        servo3_textbox.Clear()
        servo3_textbox.AppendText(TrackBar4.Value)
        serialPort.Write("SV 3 " & servo3_textbox.Text & vbCr)
    End Sub
End Class