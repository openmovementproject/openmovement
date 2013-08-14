Imports OmApiNet

Public Class Example

    Private Sub Run()

        Dim ret As Integer

        ' Start up the API
        ret = OmApi.OmStartup(OmApi.OM_VERSION)
        Debug.Print("OmStartup() = " & ret)

        ' Get number of devices
        Dim devices(0) As Integer
        ret = OmApi.OmGetDeviceIds(devices, 0)
        Debug.Print("OmGetDeviceIds() = " & ret)

        ' Get device IDs
        Dim numDevices As Integer
        numDevices = ret
        ReDim devices(numDevices)
        ret = OmApi.OmGetDeviceIds(devices, devices.Length)
        Debug.Print("OmGetDeviceIds() = " & ret)
        If ret > devices.Length Then ret = devices.Length ' ignore any devices added since last call
        numDevices = ret

        ' For each device
        For i = 0 To numDevices - 1
            Debug.Print("Device-" & (i + 1) & " = " & devices(i))

            ' Set device LED to blue
            ret = OmApi.OmSetLed(devices(i), OmApi.OM_LED_STATE.OM_LED_BLUE)
            Debug.Print("OmSetDeviceLed() = " & ret)
        Next i

        ' Shutdown the API
        ret = OmApi.OmShutdown()
        Debug.Print("OmShutdown() = " & ret)
    End Sub

    Private Sub ButtonRun_Click(sender As System.Object, e As System.EventArgs) Handles ButtonRun.Click
        Run()
    End Sub

End Class
