# PiceaToLoxoneC++ Docker-Paket für Raspberry Pi

Dieses Paket enthält:

- dein aktuelles Projekt im Ordner `PiceaToLoxoneC++`
- eine fertige `Dockerfile`
- eine fertige `compose.yaml`
- eine `.env` mit Standardwerten
- eine Beispiel-`config/config.cfg`
- einen Ordner `logs`, der auf dem Host bleibt

## Empfohlenes Zielsystem

- Raspberry Pi 5 mit 64-Bit OS
- Docker und Docker Compose Plugin installiert

## Ordner vorbereiten

Paket entpacken, dann in den Paketordner wechseln.

## Wichtige Dateien bearbeiten

### 1. `config/config.cfg`
Mindestens diese Werte anpassen:

- `LxIP` = IP deiner Loxone
- `LxUser`
- `LxPW`
- `PiceaID`
- `PiceaIP`
- `PiceaPort`
- `PiceaJWT`

Wichtige Defaults für Docker:

- `PollIntervalSeconds=1`
- `HttpPort=8080`
- `HttpBindAddress=0.0.0.0`

Hinweis:

- `LxIP` wird gleichzeitig als erlaubte Client-IP für den internen HTTP-Server verwendet.
- `HttpBindAddress` sollte im Container auf `0.0.0.0` bleiben.

### 2. `.env`
Standardmäßig:

```env
HTTP_PORT=8080
LOG_DEBUG=0
```

Wenn du `HttpPort` in der `config.cfg` änderst, ändere `HTTP_PORT` in der `.env` auf denselben Wert.

## Starten

```bash
docker compose build
mkdir -p logs
docker compose up -d
```

## Logs ansehen

Docker-Logs:

```bash
docker compose logs -f
```

Datei-Logs deines Programms:

```bash
ls -lah logs
```

## Stoppen

```bash
docker compose down
```

## Neustart nach Änderungen

Wenn du Quellcode oder Dockerfile änderst:

```bash
docker compose build --no-cache
docker compose up -d
```

Wenn du nur `config/config.cfg` änderst:

```bash
docker compose restart
```

## Wichtige Hinweise

- Die App schreibt nach `./logs` auf dem Host.
- Der Container startet automatisch neu durch `restart: unless-stopped`.
- Beim Image-Build wird `civetweb` automatisch von GitHub in den Build-Container geladen.
  Deshalb braucht der Raspberry Pi beim ersten Build Internetzugang.
- Der freigegebene Port kommt aus `.env` und muss zum `HttpPort` in `config.cfg` passen.

## Schneller Start

```bash
cd PiceaToLoxoneC++_RaspberryPi_Docker_Paket
mkdir -p logs
nano config/config.cfg
docker compose build
docker compose up -d
docker compose logs -f
```
