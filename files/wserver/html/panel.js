const menu=document.getElementById('menu');
const btn_menu=document.getElementById('nav_menu');
const btn_sys=document.getElementById('nav_sys');
const btn_sens=document.getElementById('nav_sens');
const btn_prot=document.getElementById('nav_prot');
const btn_net=document.getElementById('nav_net');
const btn_dash=document.getElementById('nav_dash');
const div_main=document.getElementById('main');

const btn_test=document.getElementById('test'); //Borrar

btn_menu.addEventListener('click', () => {
    menu.classList.toggle('is-hidden');
});

btn_sys.addEventListener('click', () => {

    menu.innerHTML=
    `
    <p class="menu-label">
    System
  </p>
  <ul class="menu-list">
    <li><a>Login</a></li>
    <li><a>Node</a></li>
    <li><a></a></li>
  </ul>
    `
});

btn_dash.addEventListener('click', () => {
    //alert('Hola')
    menu.innerHTML=
    `
    <p class="menu-label">
    Dashboard
  </p>
  <ul class="menu-list">
    <li><a>View</a></li>
    <li><a>Settings</a></li>
  </ul>
    `
});

btn_sens.addEventListener('click', () => {
    menu.innerHTML=
    `
    <p class="menu-label">
    Sensors
  </p>
  <ul class="menu-list">
    <li><a>Wired</a></li>
    <li><a>Battery</a></li>
    <li><a>Wireless</a></li>
  </ul>
    `
});

btn_prot.addEventListener('click', () => {
    //alert('Hola')
    menu.innerHTML=
    `
    <p class="menu-label">
    Protocols
  </p>
  <ul class="menu-list">
    <li><a>MQTT</a></li>
    <li><a>Websocket Client</a></li>
    <li><a>Websocket Server</a></li>
  </ul>
    `
});

btn_net.addEventListener('click', () => {
    //alert('Hola')
    menu.innerHTML=
    `
    <p class="menu-label">
    Networks
  </p>
  <ul class="menu-list">
    <li><a>Ethernet</a></li>
    <li><a>Station</a></li>
    <li><a>Acces Point</a></li>
  </ul>
    `
});


btn_test.addEventListener('click', ()=>{
    
    div_main.innerHTML=`
    <h1 class="title">Ethernet Settings</h1>
    <p class="subtitle">IPv4 Settings</p>
    
    <div class="columns is-fullwidth box">
    <div class="column is-2">
    <p class="subtitle is-4">IP address</p>
    </div>
    <div class="column is-3" >
    <input class="input is-primary subtitle is-4" type="text" placeholder="IPv4 address">
    </div>

    ip_info;
    dns[3];
    dhcp;
    ip6_addr;
    ip6_type;
    dns6[3];

</div>
    `
});
