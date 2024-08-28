import wmi

c = wmi.WMI()

for processor in c.Win32_Processor():
    print(f"Processor Name: {processor.Name}")
    print(f"Max Clock Speed: {processor.MaxClockSpeed} MHz")
    print(f"Current Clock Speed: {processor.CurrentClockSpeed} MHz")
