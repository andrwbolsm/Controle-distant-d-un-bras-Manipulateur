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
# Figure 1 : Latence (Sauvegarde en PNG)
# ==============================
fig1, ax1 = plt.subplots(figsize=(10, 6))
ax1.plot(df["tempo_envio_s"], df["latencia_ms"], color='royalblue', linewidth=1.5)
ax1.set_xlabel("Temps d'envoi (s)")
ax1.set_ylabel("Latence (ms)")
ax1.set_title("Analyse de la Latence de Communication UDP")
ax1.grid(True, linestyle='--', alpha=0.7)

fig1.savefig("graphique_latence.png", dpi=300, bbox_inches='tight')
plt.close(fig1) # Ferme la figure pour libérer la mémoire RAM

# ==============================
# Figure 2 : Valeurs q[0] (Sauvegarde en PNG)
# ==============================
fig2, ax2 = plt.subplots(figsize=(10, 6))
ax2.plot(df["tempo_envio_s"], df["q0_in"], 'r--', label="Envoyé (q0_in)", alpha=0.8)
ax2.scatter(df["tempo_envio_s"], df["q0_rcv"], 
            color='green', 
            label="Reçu (q0_rcv)", 
            s=10,         # Size of the dots
            alpha=0.6,    # Transparency (useful if points overlap)
            marker='o')   # Shape of the points
ax2.set_xlabel("Temps (s)")
ax2.set_ylabel("Valeur q[0]")
ax2.set_title("Comparaison : Données Envoyées vs Reçues")
ax2.legend()
ax2.grid(True, alpha=0.3)

fig2.savefig("comparaison_valeurs.png", dpi=300, bbox_inches='tight')
plt.close(fig2)

# ==============================
# Figure 3 : Erreur Relative (Sauvegarde en PNG)
# ==============================
fig3, ax3 = plt.subplots(figsize=(10, 6))
# epsilon pour éviter division par zéro
erreur_rel_perc = (df["q0_rcv"] - df["q0_in"]) / (df["q0_in"] + 1e-9) * 100

ax3.plot(df["tempo_envio_s"], erreur_rel_perc, color='darkorange')
ax3.set_xlabel("Temps (s)")
ax3.set_ylabel("Erreur relative (%)")
ax3.set_title("Évolution de l'Erreur Relative au cours du temps")
ax3.grid(True, linestyle=':')

fig3.savefig("erreur_relative.png", dpi=300, bbox_inches='tight')
plt.close(fig3)

print("Les graphiques ont été sauvegardés avec succès dans le dossier courant.")