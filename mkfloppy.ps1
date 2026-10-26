# mkfloppy.ps1 — build a 1.44 MB floppy image: bootsector + kernel
$img = New-Object byte[] (1440 * 1024)
$b = [IO.File]::ReadAllBytes('boot/boot.bin')
$k = [IO.File]::ReadAllBytes('kernel.bin')
$b.CopyTo($img, 0)
$k.CopyTo($img, 512)
[IO.File]::WriteAllBytes('floppy.img', $img)
Write-Output ("floppy.img written: boot={0} bytes, kernel={1} bytes" -f $b.Length, $k.Length)
