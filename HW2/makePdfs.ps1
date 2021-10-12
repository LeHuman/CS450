
Remove-Item 'PA2.pdf'
$proc0 = Start-Process -PassThru -NoNewWindow -FilePath "pandoc" -ArgumentList "PA2.md", "-o PA2.pdf"

Remove-Item 'README.pdf'
$proc1 = Start-Process -PassThru -NoNewWindow -FilePath "pandoc" -ArgumentList "README.md", "-o README.pdf"

$proc0.WaitForExit();
$proc1.WaitForExit();

# if ($proc0.ExitCode -eq 0) {
# Start-Process ((Resolve-Path "PA2.pdf").Path)
# }