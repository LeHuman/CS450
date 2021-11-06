
Remove-Item 'PA3.pdf'
$proc0 = Start-Process -PassThru -NoNewWindow -FilePath "pandoc" -ArgumentList "PA3.md", "-o PA3.pdf"

Remove-Item 'README1.pdf'
$proc1 = Start-Process -PassThru -NoNewWindow -FilePath "pandoc" -ArgumentList "README1.md", "-o README1.pdf"

Remove-Item 'README2.pdf'
$proc2 = Start-Process -PassThru -NoNewWindow -FilePath "pandoc" -ArgumentList "README2.md", "-o README2.pdf"

$proc0.WaitForExit();
$proc1.WaitForExit();
$proc2.WaitForExit();