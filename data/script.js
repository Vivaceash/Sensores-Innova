async function fetchSensores() {
    try {
      const res = await fetch('/sensors');
      if (!res.ok) throw new Error("Error al obtener datos");
      const data = await res.json();
  
      document.getElementById('temp').textContent = data.temp;
      document.getElementById('hum').textContent = data.hum;
      document.getElementById('presion').textContent = data.presion;
      document.getElementById('altitud').textContent = data.altitud;
      document.getElementById('fecha').textContent = `${data.day}/${data.month}/${data.year}`;
      document.getElementById('hora').textContent = `${data.hour}:${data.minute}:${data.second}`;
    } catch(e) {
      console.error(e);
    }
  }
  
  async function cambiarPreset(num) {
    try {
      const res = await fetch(`/preset?color=${num}`);
      if (!res.ok) alert("Error al cambiar color");
    } catch(e) {
      alert("Error: " + e.message);
    }
  }
  
  // Actualizar sensores cada 5 segundos
  setInterval(fetchSensores, 5000);
  fetchSensores();
  