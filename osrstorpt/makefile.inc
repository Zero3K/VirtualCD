$(O)\mp.bmf:
  mofcomp -WMI -B:$(O)\mp.bmf mp.mof

wmi.h: $(O)\mp.bmf
  wmimofck -h.\inc\mpwmi.h -m -w.\html $(O)\mp.bmf

