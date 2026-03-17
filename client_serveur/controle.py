import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit, fsolve

def second_order_function(t, K, zeta, wn):
    u_val = -4.0  #reference

    # Using the standard 2nd order step response formula
    if zeta < 1:  # Underdamped
        wd = wn * np.sqrt(1 - zeta**2)
        phi = np.arccos(zeta)
        return K * u_val * (1 - (1/np.sqrt(1-zeta**2)) * np.exp(-zeta*wn*t) * np.sin(wd*t + phi))
    else:  # Overdamped
        r1 = -wn * (zeta - np.sqrt(zeta**2 - 1))
        r2 = -wn * (zeta + np.sqrt(zeta**2 - 1))
        if abs(zeta - 1.0) < 1e-5:
            return K * u_val * (1 - (1 + wn*t) * np.exp(-wn*t))
        else:
            return K * u_val * (1 - (r2*np.exp(r1*t) - r1*np.exp(r2*t))/(r2 - r1))



def equation(Kc, G, Kd, Kp, Trc):
    Kc_G = Kc * G

    delta = (Kd**4)/(Kc_G ** 2) - 4*(Kp*(Kd**2)/(Kc_G**2) - 1)

    cos_theta = (1/2) * (-Kd**2 / (Kc_G) + np.sqrt(delta))

    sol = Kd * np.arccos(cos_theta) / (Kc_G * np.sqrt(1 - cos_theta**2))
    print(sol)

    return sol - Trc 

#Load and Normalize
df = pd.read_csv('latence.csv')
t = (df['t_rcv'] - df['t_rcv'].iloc[0]).values / 1e6
y = df['q0_rcv'].values
Trc = df['latence_us'].mean() / 1e6

p0 = [1.0, 0.6, 9.0] #K, zeta, wn
popt, _ = curve_fit(second_order_function, t, y, p0=p0)
K, zeta, wn = popt
tau = 1.0 / (zeta * wn)

Kd = 2*zeta*wn
Kp = wn**2
G = K*wn**2

print(f"--- 2nd Order Fit ---")
print(f"Gain (K): {K:.4f}")
print(f"Damping (zeta): {zeta:.4f}")
print(f"Natural Freq (wn): {wn:.2f} rad/s")
print(f"Time Constant (Tau): {tau:.6f} s")
print("-" * 35)
print(f"Trc : {Trc:.3f}")
print(f"Kd : {Kd:.2f}")
print(f"Kp : {Kp:.2f}")
print(f"G  : {G:.2f}")

solution = fsolve(equation, x0 = 1.1, args = (G, Kd, Kp, Trc))
Kc = solution[0]

print(f"K : {Kc}")

# t_plot = np.linspace(0, t.max(), 1000)
# y_plot = second_order_function(t_plot, *popt)

# plt.figure(figsize=(10, 6))
# plt.scatter(t, y, color='green', s=15, alpha=0.5, label='Data (q0_rcv)')
# plt.plot(t_plot, y_plot, 'b-', linewidth=2, label=f'2nd Order Fit (Tau={tau:.3f}s)')

# plt.title('Second-Order Identification (Assuming Zero Delay)')
# plt.xlabel('Time (s)')
# plt.ylabel('Response')
# plt.legend()
# plt.grid(True, alpha=0.3)
# plt.show()
