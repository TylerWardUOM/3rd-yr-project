# Use pdflatex
$pdflatex = 'pdflatex -synctex=1 -interaction=nonstopmode %O %S';

# Keep viewer open and re-render in place
$pdf_mode = 1;

# Windows-friendly viewer command (SumatraPDF recommended)
$pdf_previewer = 'start SumatraPDF.exe -reuse-instance -forward-search %f %l %r';

# Avoid endless loops
$max_repeat = 5;

