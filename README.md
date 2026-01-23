![KAYZE Banner](KayzeLauncher/banner.svg)

<div align="center">
  <a href="#-english-version">🇺🇸 <b>ENGLISH VERSION</b></a> | 
  <a href="#-versione-italiana">🇮🇹 <b>VERSIONE ITALIANA</b></a>
</div>

---

<a id="-english-version"></a>
# 🇺🇸 Discord Quest Completer by KAYZE

**Discord Quest Completer** is a lightweight, portable, open-source C++ tool designed to complete **Discord Quests** (to earn Orbs, Avatar Decorations, or in-game items) without downloading or installing massive games.

It uses an advanced **"Clean Launch"** technology to simulate game execution, tricking Discord's activity detection perfectly and allowing you to claim rewards in minutes.

---

## ✨ Features

* **🚀 No Downloads Required:** Complete quests for games like *Genshin Impact*, *Fortnite*, or *Call of Duty* without installing 100GB of data.
* **☁️ Cloud Database:** The game list is fetched directly from the **GitHub Repository** at startup. No app updates needed for new games!
* **🛡️ Clean Launch Technology (v1.0.3+):** Unlike other tools, KAYZE runs games in isolated folders (`fake_games/`) with real configuration files. This ensures **100% detection rate** by Discord.
* **🧹 Auto-Cleanup:** The app automatically cleans up temporary executables and config files when you stop the game.
* **🎨 Modern Interface:** Clean **Dark Mode** GUI based on ImGui, with custom fonts and responsive layout.
* **📦 Portable & Stealth:** A single standalone `.exe` file. No installation, no junk files, no external dependencies.

---

## 📥 Download & Install

1.  Go to the **[Releases](../../releases)** section of this repository.
2.  Download the latest `KayzeLauncher.exe`.
3.  Run the file (no installation required).

> **Note:** Since the program simulates system processes, some antiviruses might flag it as a false positive. The code is open source and can be audited by anyone. **Run as Administrator** if the quest is not detected.

---

## 🎮 Guide: How to Get Orbs & Rewards

To make Discord count the minutes for the quest, you must follow these exact steps.

### 1. Activate the Quest
* Open Discord.
* Go to **QUESTS**.
* Find the active quest (e.g., "Play 15 minutes of *Genshin Impact*") and click **Accept Quest**.

### 2. Start KAYZE
* Open `KayzeLauncher.exe`.
* Search for the game name in the search bar.
* Click **START** next to the correct executable.
* *A black window with the game title will open. **DO NOT CLOSE IT** until finished.*

### 3. Claim Reward
* Keep the stream running for **15 minutes** (or the required time).
* You can check progress by hovering over the quest.
* Once it hits 100%, click **Claim Reward**.
* Now you can click **STOP** in KAYZE and close everything.

---

## 🛠️ Build from Source

If you are a developer and want to compile the project yourself:

### Prerequisites
* **Visual Studio 2022** (Workload: C++ Desktop Development).
* Windows 10/11 SDK.

### Build Instructions
1.  Clone the repository:
    ```bash
    git clone https://github.com/kayzedevx0/KAYZE-Discord-Quest-Completer.git
    ```
2.  Open `KAYZE Discord Quest Completer.sln`.
3.  Set configuration to **Release** and **x64**.
4.  Go to *Project Properties* -> *C/C++* -> *Code Generation* and set **Runtime Library** to **Multi-threaded (/MT)** (for standalone build).
5.  Disable *Whole Program Optimization* if necessary to avoid linking conflicts.
6.  Press **Build Solution**. The executable will be in `x64/Release`.

---

## 🌐 Game Database

The program downloads the game list from the **Main Repository**. This allows adding new games without recompiling the program.

Database format (`gamelist.json`):

```json
{
  "name": "Game Name",
  "id": "DISCORD_APPLICATION_ID",
  "executables": [{"name": "game.exe", "os": "win32"}]
}
```

## ⚠️ Disclaimer

This software is provided "as is" for educational purposes. The author assumes no responsibility for any consequences to your Discord account or the simulated games. Use this tool responsibly.


---


<a id="-versione-italiana"></a>
# 🇮🇹 Discord Quest Completer by KAYZE

**Discord Quest Completer** è un tool leggero, portatile e open-source scritto in C++ progettato per completare le **Discord Quests** (per ottenere Orbs, Decorazioni Avatar o oggetti in-game) senza dover scaricare o installare giochi pesanti.

Utilizza una tecnologia avanzata di **"Clean Launch"** per simulare l'esecuzione dei giochi, garantendo il rilevamento attività di Discord al 100% e permettendoti di riscattare le ricompense in pochi minuti.

---

## ✨ Funzionalità

* **🚀 Nessun Download Richiesto:** Completa le quest di giochi come *Genshin Impact*, *Fortnite* o *Call of Duty* senza installare 100GB di dati.
* **☁️ Database Cloud:** La lista dei giochi viene scaricata automaticamente all'avvio dalla **Repository GitHub** principale. Non c'è bisogno di aggiornare l'app per avere i nuovi giochi!
* **🛡️ Tecnologia Clean Launch (v1.0.3+):** A differenza di altri tool, KAYZE avvia i giochi in cartelle isolate (`fake_games/`) utilizzando file di configurazione reali. Questo garantisce che Discord rilevi sempre il gioco.
* **🧹 Pulizia Automatica:** L'app elimina automaticamente i file `.exe` temporanei e le config quando stoppi il gioco.
* **🎨 Interfaccia Moderna:** GUI pulita in **Dark Mode** basata su ImGui, con font personalizzati e layout responsive.
* **📦 Portatile & Stealth:** Un singolo file `.exe` standalone. Nessuna installazione, nessun file spazzatura, nessuna dipendenza esterna.

---

## 📥 Download & Installazione

1.  Vai alla sezione **[Releases](../../releases)** di questa repository.
2.  Scarica l'ultimo file `KayzeLauncher.exe`.
3.  Esegui il file (non richiede installazione).

> **Nota:** Poiché il programma simula processi di sistema, alcuni antivirus potrebbero segnalarlo come falso positivo. Il codice è open source e può essere controllato da chiunque. Esegui come **Amministratore** se la quest non viene rilevata.

---

## 🎮 Guida: Come ottenere Orbs e Ricompense

Per far sì che Discord conti i minuti per la quest, devi seguire questi passaggi precisi.

### 1. Attiva la Quest
* Apri Discord.
* Vai in **MISSIONI**.
* Cerca la quest attiva (es. "Gioca 15 minuti a *Genshin Impact*") e clicca su **Accetta Quest**.

### 2. Avvia KAYZE
* Apri `KayzeLauncher.exe`.
* Cerca il nome del gioco nella barra di ricerca.
* Clicca su **START** accanto all'eseguibile corretto.
* *Si aprirà una finestra nera con il titolo del gioco. **NON CHIUDERLA** fino alla fine.*

### 3. Riscatta
* Lascia lo stream attivo per **15 minuti** (o il tempo richiesto dalla quest).
* Puoi controllare l'avanzamento passando il mouse sulla quest.
* Una volta arrivato al 100%, clicca su **Riscatta Ricompensa**.
* Ora puoi cliccare su **STOP** in KAYZE e chiudere tutto.

---

## 🛠️ Compilare dai Sorgenti

Se sei uno sviluppatore e vuoi compilare il progetto da solo:

### Prerequisiti
* **Visual Studio 2022** (Workload: Sviluppo desktop con C++).
* SDK di Windows 10/11.

### Istruzioni Build
1.  Clona la repository:
    ```bash
    git clone https://github.com/kayzedevx0/KAYZE-Discord-Quest-Completer.git
    ```
2.  Apri il file `KAYZE Discord Quest Completer.sln`.
3.  Imposta la configurazione su **Release** e **x64**.
4.  Vai su *Project Properties* -> *C/C++* -> *Code Generation* e imposta **Runtime Library** su **Multi-threaded (/MT)** (per renderlo standalone).
5.  Disabilita *Whole Program Optimization* se necessario per evitare conflitti di link.
6.  Premi **Build Solution**. L'eseguibile sarà in `x64/Release`.

---


# 🌐 Database Giochi

Il programma scarica la lista dei giochi dalla **Repository GitHub**. Questo permette di aggiungere nuovi giochi senza dover ricompilare il programma.

Il formato del database è:

```json
{
  "name": "Nome Del Gioco",
  "id": "DISCORD_APPLICATION_ID",
  "executables": [{"name": "gioco.exe", "os": "win32"}]
}
```

## ⚠️ Disclaimer

Questo software è fornito "così com'è" a scopo educativo. L'autore non si assume alcuna responsabilità per eventuali conseguenze sul tuo account Discord o sui giochi simulati. Utilizza questo strumento responsabilmente.

## ❤️ Crediti

*   Sviluppato da: **KAYZE**
*   UI Library: Dear ImGui
*   JSON Parser: nlohmann/json
*   Font: Rajdhani (Embedded)

<p align="center"> <i>Se questo tool ti è stato utile, lascia una ⭐ alla repository!</i> </p>
