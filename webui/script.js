async function loadStatus() {
    const res = await fetch("/api/status");
    const st = await res.json();
    document.getElementById("dhcp").checked = st.dhcp;
    document.getElementById("ip").value = st.ip;
    document.getElementById("netmask").value = st.netmask;
    document.getElementById("gateway").value = st.gateway;
    document.getElementById("telnet").checked = st.telnet;
}

async function saveNetwork(e) {
    e.preventDefault();
    const data = new URLSearchParams();
    data.append("dhcp", document.getElementById("dhcp").checked);
    data.append("ip", document.getElementById("ip").value);
    data.append("netmask", document.getElementById("netmask").value);
    data.append("gateway", document.getElementById("gateway").value);
    data.append("telnet", document.getElementById("telnet").checked);

    await fetch("/api/set", {
        method: "POST",
        body: data.toString(),
    });
    alert("Gespeichert. Gerät startet ggf. neu.");
}

async function resetDevice() {
    if (!confirm("Werkseinstellungen laden?")) return;
    await fetch("/api/reset", { method: "POST" });
    alert("Zurückgesetzt – Gerät startet neu.");
}

async function sendCommand(cmd) {
    // UART-Kommandos an Bridge-Port (8080)
    try {
        const sock = new WebSocket("ws://"+location.hostname+":8080");
        sock.onopen = () => sock.send(cmd+"\n");
        sock.onmessage = ev => {
            document.getElementById("output").textContent += ev.data + "\n";
        };
    } catch (e) {
        alert("Fehler beim Senden: " + e);
    }
}

function showTab(name) {
    document.querySelectorAll(".tab").forEach(tab => tab.classList.add("hidden"));
    document.getElementById(name).classList.remove("hidden");
}

window.onload = loadStatus;
