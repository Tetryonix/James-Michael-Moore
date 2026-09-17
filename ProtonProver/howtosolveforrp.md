v$5.1$

James M. Moore
9/4/2026
License: AGPL 3.0-or-later

 _Thank you for downloading my proton prover.  This program is designed specifically to test the quantum scale wavelength for an equilateral boson with the proton radius using Kelvin C. Abraham's Tetryonic Theory.


This guide calculates two geometric averages from Kelvin C. Abraham’s proposed Tetryonics proton geometry and compares one of the model results with the CODATA 2022 proton-radius value.  Whenever the dashboard lights up, the theory has converged to experimental prediction.  This does not mean that the total average of all tests a user may perform will match CODATA.  But I hope that in general the convergences users achieve will prove that the correlation is perhaps not a coincidence.


| **Symbol**                                    | **Description**                                                                                  |
| --------------------------------------------- | ------------------------------------------------------------------------------------------------ |
| $N$                                           | Total number of samples selected.                                                                |
| $i$                                           | Current sample number, from $1$ through $N$.                                                     |
| $j$                                           | STL triangle-face index, from $1$ through $20$.                                                  |
| $p_i$                                         | The selected sampled surface point, $p_i = (x_i, y_i, z_i)$.                                     |
| $h$                                           | The center or hub, $h = (x_h, y_h, z_h)$, shifted to $(0,0,0)$.                                  |
| $r_i$                                         | Distance from the hub to sampled surface point $p_i$.                                            |
| $S_N$                                         | Arithmetic mean of $N$ distances.                                                                |
| $S$                                           | The current or converged value of $S_N$.                                                         |
| $x_k, y_k, z_k$                               | Centered and scaled 3D coordinates of STL vertex $k$.                                            |
| $x'_{i,k}, y'_{i,k}, z'_{i,k}$                | Rotated coordinates of vertex $k$ in viewing direction $i$; $x'$ and $y'$ form the screen plane. |
| $k$                                           | One of the 12 STL vertices checked in one projected view (vertex index).                         |
| $k_i^*$                                       | Index of the farthest projected vertex in viewing direction $i$.                                 |
| $\text{Vertex}_{i,k}$                         | Projected center-to-vertex distance for vertex $k$ in viewing direction $i$.                     |
| $R_{\text{View},i}$                           | Largest center-to-vertex distance in the current 2D projection.                                  |
| $R_{\text{STL}}$                              | Fixed maximum three-dimensional hub-to-vertex radius of the STL.                                 |
| $g_i$                                         | Gap $R_{\text{STL}} - R_{\text{View},i}$ for viewing direction $i$.                              |
| $G_N$                                         | Arithmetic mean of $N$ gaps, equal to $R_{\text{STL}} - A_N$.                                    |
| $A_N$                                         | Converged arithmetic mean of the $N$ projected radii $R_{\text{View},i}$.                        |
| $A$                                           | The current or converged value of $A_N$.                                                         |
| $R_x, R_z$                                    | Rotation matrices about the x-axis and z-axis.                                                   |
| $\text{yaw}_i, \text{pitch}_i, \text{roll}_i$ | Rotation angles defining viewing direction $i$.                                                  |
| $r_{\text{CODATA}}$                           | CODATA 2022 proton rms charge-radius central value.                                              |
| $\Delta_N$                                    | Signed difference $S_N - r_{\text{CODATA}}$.                                                     |
| $\sigma_{\text{CODATA}}$                      | CODATA standard uncertainty, $0.00064\text{ fm}$.                                                |
| $\sigma_{S_N}$                                | Signed difference $\Delta_N$ expressed in CODATA standard-uncertainty units.                     |
| $v_{\text{native}}, h_{\text{native}}$        | A raw STL vertex and the raw STL hub coordinate before centering and scaling.                    |
| $v_{\text{centered,fm}}$                      | The corresponding centered and scaled vertex coordinate in femtometres.                          |
| $\text{argmax}$                               | The index of the tested value that is largest.                                                   |
| $f$                                           | Scale factor, $.041000061903\frac{fm}{native unit}$                                              |

The STL file's original coordinate origin is arbitrary, so its central hub is not actually stored at $(0,0,0)$.  This is normal and does not alter the geometry.

Before sampling or measuring anything, the program subtracts the raw hub coordinate from every STL vertex and then applies the scale: 

$$(v_{native} - h_{native}) \times 0.041000061903 \frac{fm}{native unit} = v_{centered}fm$$
The program centers the mesh using an ordinary translation:

$$p_i^{centered} = p_i^{native} - h^{native}$$
and then scales it by the factor $f$:

$$p_i^{fm} = (p_i^{native} - h^{native})f$$

Consequently, the hub used by every displayed coordinate and every calculation is exactly $h=(0,0,0)$.  The face-vertex coordinates and $p_i$ values shown in this guide are already centered and scaled.  Native units multiplied by $\frac{fm}{native}$ produces the femtometre scale.


## Calculating the classical surface average S(the charge distribution radius)

### 1. Set the empirical measurement for the proton radius:

[.841fm CODATA 2022]([CODATA Value: proton rms charge radius](https://physics.nist.gov/cgi-bin/cuu/Value?rp)).

### 2. Define the proton geometry:

Use Kelvin C. Abraham’s Tetryonics proton geometry represented by the 20-face [.STL](linktokelvinsstl).

### 3. Scale fix the fascia height:

The Tetryon Scale([10.5281/zenodo.22107938]([The Tetryon Scale | Zenodo](https://zenodo.org/records/22159340)))

### 4.  Set the origin as the center of mass:

Assign the hub's coordinates:
$$h = (0, 0, 0)$$

### 5. Define a coordinate on the surface of a triangle(j):

Fetch your sample coordinates $(x,y,z)$ from the STL using manual mode.

$$p_i=(x_i,y_i,z_i).$$
### 6.  Formulate the distance from the center of mass:

Plug the coordinates $p_i$ into the Euclidean distance formula:

$$r_i = \lVert p_i-h\rVert = \sqrt{(x_i-x_h)^2+(y_i-y_h)^2+(z_i-z_h)^2}$$

since $x_h$, $y_h$, and $z_h$ are equal to $0$,

$$r_i = ||p_i|| =  \sqrt{x_i^2 + y_i^2 + z_i^2}$$

### 7. Sample uniformly distributed points and take the arithmetic mean:

Evaluate the iterated sum over the number of iterations:

$$S_N = \frac{1}{N}\sum_{i=1}^{N} r_i = \frac{r_1 + r_2 + ... + r_N}{N}$$

### 8.  Compare to CODATA:
Take the difference between the sample average and the CODATA average:
$$\Delta_N = S_N - r_{CODATA}$$

### 9.  Calculate the uncertainty:

$$\sigma_{CODATA} = 0.00640 \times 10^{-16}m$$

$$\sigma_{S_N} = \frac{\Delta_N}{\sigma_{CODATA}} = \frac{S_N - .84075fm}{.00064fm} = \frac{S_N}{.000640fm} -\frac{r_{CODATA}}{.000640fm}$$

$$\frac{0.840750}{0.000640}=1313.67188$$

$$\sigma_{S_N} = \frac{S_N}{.000640fm} - 1313.67188$$

---

## Calculating the average distance to the farthest exterior samples(the 'true' geometric radius)

$A$ is the average distance from the center of the proton to specifically the farthest exterior point on any given group of samples.  Using a faceted geometry will change the radius depending on its viewing angle.  For each iteration, you take the point closest to the maximum possible physical extent of the proton in a solid of revolution.  This, by the nature of Kelvin's design, is inherently equal to the side length of a tetrahedron.  Then, you measure the distance between the center and that point. You repeat this process over many iterations to get $A$, which I call the 'true' proton's average geometric radius.

### 1. Look at the proton from a given direction.

$${\begin{bmatrix}x'_{i,k} \\y'_{i,k}\\z'_{i,k} \end{bmatrix} = R_z(\mathrm{roll}_i) R_x(\mathrm{pitch}_i) R_z(\mathrm{yaw}_i) \begin{bmatrix} x_k\\y_k\\z_k \end{bmatrix}}$$

### 2.  First calculate the projected distance for every vertex:

$$Vertex_{i,k} = \sqrt{x_{i,k}'^2 + y_{i,k}'^2}$$

### 3. Define the coordinates for the point farthest from the center of the proton.

$$k_i^* = \operatorname*{argmax}_k Vertex_{i,k}$$

### 4. Measure the distance from the center to that point.

$$R_{View, i} = \sqrt{x_{i,k_i^*}'^2 + y_{i,k_i^*}'^2}$$

### 5. Repeat over evenly distributed viewing directions.

$$R_{\mathrm{View},1}+\cdots+R_{\mathrm{View},N}$$

### 6. Add the recorded distances and divide by the number of iterations:

$$A_N = \frac{1}{N}\sum_{i=1}^{N}R_{\mathrm{View},i} = \frac{R_{\mathrm{View},1}+\cdots+R_{\mathrm{View},N}}{N}$$

Consistency check:

### 1. The gap between the average distance and the maximum distance of the farthest point in a solid of revolution is defined by:

$$G_N = R_{STL} - A_N = \frac{1}{N}\sum_{i=1}^Ng_i$$

### 2. The gap between the fixed STL outer bound and the projected radius in viewing direction $i$ is:

$$g_i = R_{STL} - R_{View, i}$$

therefore,

$$R_{View,i} = R_{STL} - g_{i}$$

$$\implies$$

$$A_N = R_{STL} - \frac{1}{N}\sum_ig_i = \frac{1}{N}\sum_{i=1}^NR_{View,i} = R_{STL} - G_N$$

They are equal only when $R_{View}$ matches the current average in the first iteration, whereafter for $N > 1$, $A_N$ becomes the running average.  Measuring the average gap between $R_{STL}$ and $R_{View}$ over many samples and subtracting it from $R_{STL}$ produces $A$.

$A$ is simply the average of all measured $R_{View}$ values.

---

> Manual mode reports the final centered and scaled Cartesian coordinate $p_i=(x_i,y_i,z_i)$. The program’s internal barycentric calculation merely identifies the clicked location within its triangle and is not required when independently checking $r_i$ or $S_N$.

Then reviewers can verify:

$$p_i=(x_i,y_i,z_i) \rightarrow r_i=\sqrt{x_i^2+y_i^2+z_i^2} \rightarrow S_N=\frac{1}{N}\sum_{i=1}^{N}r_i.$$

Manual clicks are not automatically uniformly distributed. Step 7 of calculating $S$ is valid for the automatic area-weighted sampler, but manually choosing points introduces selection bias.  The point-sweep style of the iterative form was chosen, designed, and implemented entirely by Codex.

---

AGPL3.0-or-later. To see a copy of this license, see [GNU Affero General Public License v3.0](https://choosealicense.com/licenses/agpl-3.0/).
