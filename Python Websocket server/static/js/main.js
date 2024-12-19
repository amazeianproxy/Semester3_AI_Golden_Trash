let autoRefreshInterval;

async function changeModule(){
    const response = await fetch('/next-module', {method:'POST'});
    const result = await response.json();
    document.getElementById('deviceidx').textContent = "Device " + result.device;
}

async function refreshImage(){
    const container = document.getElementById("image-container");
    const timestamp = new Date().getTime(); // Prevent caching
    const img = new Image();
    img.src = `/img?t=${timestamp}`;
    img.style.display = 'block';
    
    // Clear previous image
    container.innerHTML = '';
    container.appendChild(img);
}

async function deleteCachedDevicesWS(){
    alert("deleted");
    const response = await fetch('/del', {method:'POST'});
    const result = await response.json();
    console.log(result.message);
}

function toggleAutoRefresh(checkbox) {
    if (checkbox.checked) {
        // Start auto refresh - every 500ms
        autoRefreshInterval = setInterval(refreshImage, 500);
    } else {
        // Stop auto refresh
        clearInterval(autoRefreshInterval);
    }
}

// Wait for DOM to be fully loaded before setting up event handlers
document.addEventListener('DOMContentLoaded', () => {
    // Set up DEL button
    const delButton = document.querySelector('button[style*="background-color: rgb(172, 0, 0)"]');
    delButton.onclick = async () => {
        await deleteCachedDevicesWS();
    };

    // Set up refresh image button
    const refreshButton = document.querySelector('button:nth-of-type(3)');
    refreshButton.onclick = async () => {
        await refreshImage();
    };

    // Set up next module button
    const nextButton = document.querySelector('button:nth-of-type(1)');
    nextButton.onclick = async () => {
        await changeModule();
    };

    // Set up auto refresh checkbox
    const autoRefreshCheckbox = document.getElementById('autoRefresh');
    autoRefreshCheckbox.onchange = () => {
        toggleAutoRefresh(autoRefreshCheckbox);
    };
});

