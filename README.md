desert-copter
=============

Desert Copter è una simulazione grafica interattiva realizzata in OpenGL finalizzata a mettere in pratica la maggior parte degli argomenti trattati nel corso di Computer Graphics M.

Come suggerisce il nome, l'utente può pilotare il modellino di un elicottero civile in un'ambientazione desertica. All'interno della scena viene costantemente generata una cassa posizionata casualmente e lo scopo principale del giocatore è quello di raccoglierne il più possibile prima che il carburante si esaurisca e senza apportare troppi danni all'elicottero. L'utente è aiutato da una minimappa che mostra costantemente una porzione del terreno di gioco ed eventuali casse presenti nelle vicinanze. Ad intervalli casuali compaiono inoltre delle mongolfiere che – se raccolte – possono ricompensare il giocatore con un bonus in carburante oppure riparare parte dei danni subiti.

La realizzazione del progetto ha permesso di sperimentare e lavorare in particolar modo sui seguenti argomenti:
vista tridimensionale interattiva
strutture per la gestione di modelli 3D
texture mapping
illuminazione e materiali
input utente
shading e blending
stencil buffer
environmental mapping
accumulation buffer
Questa documentazione ha lo scopo di illustrare l'organizzazione complessiva del progetto, le principali tecniche utilizzate per la resa grafica, le scelte affrontate durante la progettazione e le problematiche affrontate nella realizzazione. Le aree tematiche che non sono di interesse per il corso quali i dettagli sulla logica di gioco, simulazione fisica, gestione delle collisioni e strutture dati accessorie verranno accennate solo superficialmente.

Il progetto è stato realizzato in ambiente Linux utilizzando il linguaggio C (specifica gnu99), le librerie GL, GLU per l'accesso alle funzionalità grafiche e la libreria SDL per la gestione della superficie di rendering e dell'input utente.
Per la creazione/modifica dei modelli poligonali (e relativi materiali) sono stati utilizzati 3ds MAX e Blender, mentre per la messa a punto delle numerose texture di gioco si è fatto largo uso del pacchetto grafico Gimp.
Tutti i modelli 3D utilizzati e buona parte delle texture sono state ricavate da http://www.turbosquid.com.

Desert Copter è ispirato al (vetusto) videogioco SimCopter della Maxis; questo progetto vuole esserne un affezionato omaggio.

ISTRUZIONI DI COMPILAZIONE:

Per compilare il progetto seguire i seguenti passi:
- portarsi da terminale nella cartella "Debug" all'interno della cartella del progetto
- eseguire il comando make
- terminata la compilazione spostare l'eseguibile DesertCopter generato nella cartella superiore
  (mv DesertCopter ..)
- eseguire DesertCopter nella cartella principale
