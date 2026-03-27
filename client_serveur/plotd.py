import pandas as pd
import matplotlib.pyplot as plt

# ==============================
# Lecture du fichier CSV
# ==============================
arquivo = "latence.csv"
df = pd.read_csv(arquivo)

# Conversion des données
df["tempo_envio_s"] = (df["t_in"] - df["t_in"].iloc[0]) * 1e-6
df["latencia_ms"] = df["latence_us"] * 1e-3

# ==============================
# Figure 1 : Latence (PNG)
# ==============================
fig1, ax1 = plt.subplots(figsize=(10, 6))
ax1.plot(df["tempo_envio_s"], df["latencia_ms"], color='royalblue', linewidth=1.5)
ax1.set_xlabel("Temps d'envoi (s)")
ax1.set_ylabel("Latence (ms)")
ax1.set_title("Analyse de la Latence de Communication UDP")
ax1.grid(True, linestyle='--', alpha=0.7)

fig1.savefig("graphique_latence.png", dpi=300, bbox_inches='tight')
plt.close(fig1)


# ================================================
# Figures 2 à 7 : Comparaison q[i] envoyé / reçu
# ================================================
for i in range(6):
    q_in = f"q{i}_in"
    q_rcv = f"q{i}_rcv"

    fig, ax = plt.subplots(figsize=(10, 6))
    ax.plot(df["tempo_envio_s"], df[q_in], 'r--', label=f"Envoyé ({q_in})", alpha=0.8)
    ax.scatter(df["tempo_envio_s"], df[q_rcv], 
               color='green', 
               label=f"Reçu ({q_rcv})",
               s=10, alpha=0.6)

    ax.set_xlabel("Temps (s)")
    ax.set_ylabel(f"Valeur q[{i}]")
    ax.set_title(f"Comparaison : {q_in} vs {q_rcv}")
    ax.legend()
    ax.grid(True, alpha=0.3)

    fig.savefig(f"comparaison_q{i}.png", dpi=300, bbox_inches='tight')
    plt.close(fig)


# ======================================================
# Figures d’erreur relative pour q0…q5 (PNG)
# ======================================================
for i in range(6):
    q_in = f"q{i}_in"
    q_rcv = f"q{i}_rcv"

    erreur_rel_perc = (df[q_rcv] - df[q_in]) / (df[q_in] + 1e-9) * 100

    fig, ax = plt.subplots(figsize=(10, 6))
    ax.plot(df["tempo_envio_s"], erreur_rel_perc, color='darkorange')
    ax.set_xlabel("Temps (s)")
    ax.set_ylabel("Erreur relative (%)")
    ax.set_title(f"Erreur Relative pour q[{i}]")
    ax.grid(True, linestyle=':')

    fig.savefig(f"erreur_relative_q{i}.png", dpi=300, bbox_inches='tight')
    plt.close(fig)

print("✅ Tous les graphiques q0–q5 ont été générés et sauvegardés.")