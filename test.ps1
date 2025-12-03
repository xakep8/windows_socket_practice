$server = "127.0.0.1"
$port   = 5173

try {
    Write-Host "Connecting to $server`:$port..." -ForegroundColor Green
    $client = New-Object System.Net.Sockets.TcpClient($server, $port)
    $stream = $client.GetStream()
    $stream.ReadTimeout = 5000  # 5 second timeout
    
    Write-Host "Connected! Type messages and press Enter. Type 'exit' to quit." -ForegroundColor Green
    
    while ($true) {
        # Get user input
        $message = Read-Host "`nEnter message"
        
        if ($message -eq "exit" -or $message -eq "") {
            break
        }

        # Send message as bytes
        $bytes = [System.Text.Encoding]::UTF8.GetBytes($message)
        Write-Host "Sending: $message" -ForegroundColor Cyan
        
        try {
            $stream.Write($bytes, 0, $bytes.Length)
            $stream.Flush()
        }
        catch {
            Write-Host "Error sending: Connection lost." -ForegroundColor Red
            break
        }

        # Read echo response (same number of bytes we sent)
        $buffer = New-Object byte[] $bytes.Length
        $totalReceived = 0
        
        try {
            while ($totalReceived -lt $bytes.Length) {
                $bytesRead = $stream.Read($buffer, $totalReceived, $bytes.Length - $totalReceived)
                if ($bytesRead -eq 0) {
                    Write-Host "Server closed connection." -ForegroundColor Yellow
                    break
                }
                $totalReceived += $bytesRead
            }
            
            if ($totalReceived -gt 0) {
                $responseText = [System.Text.Encoding]::UTF8.GetString($buffer, 0, $totalReceived)
                Write-Host "Echo: $responseText" -ForegroundColor Green
            }
        }
        catch {
            Write-Host "Error reading response: $_" -ForegroundColor Red
            break
        }
    }
}
catch {
    Write-Host "Error: $_" -ForegroundColor Red
}
finally {
    if ($stream) { $stream.Close() }
    if ($client) { $client.Close() }
    Write-Host "`nConnection closed." -ForegroundColor Yellow
}
