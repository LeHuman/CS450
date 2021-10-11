$proc = Start-Process -PassThru -NoNewWindow -FilePath "pandoc" -ArgumentList "HW2.md", "-o Task1.pdf"
$proc.WaitForExit();
# if ($proc.ExitCode -eq 0) {
Start-Process ((Resolve-Path "Task1.pdf").Path)
# }