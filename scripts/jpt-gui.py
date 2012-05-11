from Tkinter import *

class App:

	def __init__(self, master):

		frame = Frame(master, width = 200, height = 200, bd = 10)
		frame.pack()
		
		Label(frame, text = "JPT File").grid(padx = 4, pady = 2, row = 0, sticky = W)
		self.entry_jpt = Entry(frame)
		self.entry_jpt.grid(padx = 4, pady = 2, row = 0, column = 1)
		self.button_jpt = Button(frame, text = "Browse")
		self.button_jpt.grid(padx = 4, pady = 2, row = 0, column = 2)
		
		Label(frame, text = "JPEG File").grid(padx = 4, pady = 2, row = 1, sticky = W)
		self.entry_jpeg = Entry(frame)
		self.entry_jpeg.grid(padx = 4, pady = 2, row = 1, column = 1)
		self.button_jpeg = Button(frame, text = "Browse")
		self.button_jpeg.grid(padx = 4, pady = 2, row = 1, column = 2)
		
		Label(frame, text = "PNG File").grid(padx = 4, pady = 2, row = 2, sticky = W)
		self.entry_png = Entry(frame)
		self.entry_png.grid(padx = 4, pady = 2, row = 2, column = 1)
		self.button_png = Button(frame, text = "Browse")
		self.button_png.grid(padx = 4, pady = 2, row = 2, column = 2)
		
root = Tk()
root.title("JPT GUI")
root.iconbitmap(default = 'jpt-gui.ico')
root.resizable(False, False)
app = App(root)
root.mainloop()
