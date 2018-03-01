import matplotlib.pyplot as plt
import numpy as np
import time

def make_canvases():
    fig, axes = plt.subplots(2, 5, sharex=True, sharey=True)

    plots = []
    for i in range(2):
        for j in range(5):
            plots.append(axes[i, j])

    plt.show(block=False)
    window = plt.get_current_fig_manager().window

    plt.get_current_fig_manager().resize(
        window.winfo_screenwidth(),
        window.winfo_screenheight())
    textbox = Textbox(plots[-1])

    return fig, plots[:-1], textbox

class Textbox():
    def __update_ctx(self):
        if self.ctx != None:
            self.ctx.set_visible(False)

        x0, xmax = plt.xlim()
        y0, ymax = plt.ylim()
        data_width = xmax - x0
        data_height = ymax - y0
        px = x0 + data_width * 0.15
        py = y0 + data_height * 0.5

        self.ctx = self.plot.text(
            px,
            py,
            self.text)

    def __init__(self, plot):
        self.text = ''
        self.plot = plot
        self.ctx = None

        self.__update_ctx()
        plot.axis('off')

    def set_text(self, text):
        self.text = text
        self.text += '\n'

    def append_text(self, text):
        self.text += text
        self.text += '\n'

    def append_separator(self):
        self.text += "====================\n"

    def render(self):
        self.__update_ctx()

class Canvas():
    def __init__(self, fig, ax, range_size, fig_name="default"):
        self.fig = fig
        self.ax = ax

        self.x  = range(0, range_size)
        self.y  = [0] * range_size
        self.data, = ax.plot(self.x, self.y)

        self.ax.set_xlabel("time(s)")
        self.ax.set_ylabel("pdr")
        self.ax.set_title(fig_name)
        self.fig.canvas.draw()

    def update_data(self, x, y):
        self.fig.gca().set_xlim([min(x), max(x)])
        self.fig.gca().set_ylim([0, 1.1])

        self.x = x
        self.y = y
        if len(self.y) < len(self.x):
            self.y += [0] * (len(self.x) - len(self.y))
        self.data.set_xdata(self.x)
        self.data.set_ydata(self.y)

    def draw(self):
        self.fig.canvas.draw()

    def run(self):
        pass
