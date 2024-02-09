# 2D Wave Simulation

## The Wave Equation

This is the two-dimensional wave equation which describes wave propagation.
$$
\frac{\part^2 u}{\part t^2} = c^2 
	\left( \frac{\part^2 u}{\part x^2}
    	 + \frac{\part^2 u}{\part y^2} \right)
$$

* $u$ represents the wave function $u(x, y, t)$. It can represent various quantities, such as pressure in a sound wave or the height of water waves, given a particular position and time.

* $t$ is time.
* $x$ and $y$ are the spatial coordinates.

* $c$ is the speed of wave propagation.

Starting on the left $\frac{\part^2 u}{\part t^2}$, this is the second time derivative of the wave function which represents the acceleration of the wave in time. The right side of the equation $\frac{\part^2 u}{\part x^2} + \frac{\part^2 u}{\part y^2}$ is the second spatial derivative of the wave function for each dimension. This would represent the "curvature" of the wave in space. This whole equation thus shows that the acceleration of the wave in time is governed by the curvature of the wave in space multiplied by some constant wave speed $c$​.

## Time Derivative

Since we are dealing with discrete values for both space (a grid) and time (individual frames/timesteps), we need a way to approximate these second derivatives. Essentially, given a current grid state and previous states, we want to determine what the next state/frame should be in time. For this, we will use the finite difference method which allows us to approximate derivatives given a certain number of states in either time or space.

The second-order finite difference formula for one dimension is
$$
f''(x) \approx \frac{f(x+h)-2f(x)+f(x-h)}{h^2}
$$

* $x$ is the position in space
* $h$ is the distance between discrete points in space

Since we want to find the derivative of time and not space we can adjust this formula. Instead of looking at discrete distances left and right of a position, we can look forward and backward in time. Now $h$ represents the delta timestep between states rather than between spatial grid points. To represent this better, we can say $f(x+h)$ is the state in the future $f_\text{future}(x)$, $f(x)$ is the current state $f_\text{present}(x)$, and $f(x-h)$ is the state in the past $f_\text{past}(x)$. We can also say $h$ is the delta time between states or $\Delta t$.

Now we get
$$
f''(x) \approx \frac{f_\text{future}(x) - 2f_\text{present}(x) + f_\text{past}(x)}{\Delta t^2}
$$
However, this assumes we have one spatial dimension $x$, so we can substitute $x$ in the functions for $(x, y)$​.
$$
f''(x,y) \approx \frac{f_\text{future}(x,y) - 2f_\text{present}(x,y) + f_\text{past}(x,y)}{\Delta t^2}
$$
Our goal is to determine the state of the function in the future at a specific $(x,y)$ coordinate. This means we need to solve for $f_\text{future}(x,y)$
$$
f_\text{future}(x,y) = f''(x,y)\Delta t^2 - f_\text{past}(x,y) + 2f_\text{present}(x,y)
$$
Going back to our original wave propagation equation, $\frac{\part^2 u}{\part t^2}$ is represented as $f''(x,y)$​ in our equation. They are the same, just different notation and also a partial derivative because the wave function $u(x, y, t)$ is a function of both space and time and thus just taking the derivative of time makes it a partial derivative instead of a complete derivative.
$$
f_\text{future}(x,y) = \frac{\part^2 u}{\part t^2}\Delta t^2 - f_\text{past}(x,y) + 2f_\text{present}(x,y)
$$
In our simulation, we can store the present $f_\text{present}(x,y)$ and past $f_\text{past}(x,y)$ states of our simulation in separate buffers and also keep track of the timestep $\Delta t$, however, in order to fully calculate the future state, we need the second partial derivative of the wave function with respect to time $\frac{\part^2 u}{\part t^2}$. Using the original wave propagation function, we can substitute the right side of the equation in.
$$
f_\text{future}(x,y) = c^2 
	\left( \frac{\part^2 u}{\part x^2}
    	 + \frac{\part^2 u}{\part y^2} \right)
   	\Delta t^2 
	- f_\text{past}(x,y) + 2f_\text{present}(x,y)
$$

## Spatial Derivative

We now need to approximate the second spatial derivative of the wave function $\frac{\part^2 u}{\part x^2}+\frac{\part^2 u}{\part y^2}$. We can use the same method we used for time; the finite difference method. However, unlike time where there is a single dimension, we have two spatial dimensions thus requiring us to use the two-dimensional second-order finite difference formula.
$$
f''(x,y) \approx \frac{f(x-h,y)+f(x+h,y)-4f(x,y)+f(x,y-h)+f(x,y+h)}{h^2}
$$

* $x$ and $y$ are coordinates in space
* $h$ is the distance between grid points.

Even though this equation looks pretty complicated, it can actually be visualized nicely through a stencil which can be imagined as an overlay applied on top of a grid.

==TODO==



