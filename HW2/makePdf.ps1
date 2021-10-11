$proc = Start-Process -PassThru -NoNewWindow -FilePath "pandoc" -ArgumentList "HW2.md", "-o HW2.pdf"
$proc.WaitForExit();
# if ($proc.ExitCode -eq 0) {
Start-Process ((Resolve-Path "HW2.pdf").Path)
# }