import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import time

plt.style.use('ggplot')

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

    return fig, plots[:-1]

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
        # self.fig.canvas.draw()

    def run(self, animate):
        ani = animation.FuncAnimation(self.fig, animate, frames=1000, interval=1000)
        plt.show()
