<!DOCTYPE html>
<html lang="en" dir="ltr">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>MeshLink - Protothon</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: #1a1a2e;
            background-image:
                linear-gradient(rgba(102, 126, 234, 0.2) 1px, transparent 1px),
                linear-gradient(90deg, rgba(102, 126, 234, 0.2) 1px, transparent 1px);
            background-size: 50px 50px;
            background-position: 0 0, 0 0;
            min-height: 100vh;
            padding: 20px;
            animation: gridMove 10s linear infinite;
        }

        @keyframes gridMove {
            0% {
                background-position: 0 0, 0 0;
            }

            100% {
                background-position: 0 0, 50px 50px;
            }
        }

        .container {
            max-width: 1200px;
            margin: 0 auto;
        }

        .header {
            text-align: center;
            margin-bottom: 40px;
            animation: fadeInDown 0.8s ease;
        }

        .logo {
            font-size: 3em;
            font-weight: bold;
            background: linear-gradient(45deg, #fff, #f0f0f0);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.1);
        }

        .subtitle {
            color: #fff;
            font-size: 1.2em;
            opacity: 0.7;
        }

        .stats-container {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
            margin-bottom: 30px;
            animation: fadeInUp 0.8s ease 0.2s both;
        }

        .stat-card {
            background: white;
            border-radius: 15px;
            padding: 25px;
            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.2);
            transition: transform 0.3s ease, box-shadow 0.3s ease;
        }

        .stat-card:hover {
            transform: translateY(-5px);
            box-shadow: 0 15px 40px rgba(0, 0, 0, 0.3);
        }

        .stat-header {
            display: flex;
            align-items: center;
            justify-content: center;
            margin-bottom: 20px;
        }

        .stat-icon {
            width: 80px;
            height: 80px;
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 2.5em;
            font-weight: bold;
            color: white;
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
        }

        .stat-icon.present {
            background: linear-gradient(135deg, #11998e 0%, #38ef7d 100%);
        }

        .stat-icon.absent {
            background: linear-gradient(135deg, #eb3349 0%, #f45c43 100%);
        }

        .stat-label {
            color: #666;
            margin: 10px;
            text-align: center;
        }

        .names-list {
            background: #f8f9fa;
            border-radius: 10px;
            padding: 15px;
            max-height: 300px;
            overflow-y: auto;
        }

        .names-list::-webkit-scrollbar {
            width: 6px;
        }

        .names-list::-webkit-scrollbar-track {
            background: #e0e0e0;
            border-radius: 10px;
        }

        .names-list::-webkit-scrollbar-thumb {
            background: #888;
            border-radius: 10px;
        }

        .name-item {
            padding: 12px 15px;
            background: white;
            margin-bottom: 8px;
            border-radius: 8px;
            border-right: 4px solid;
            transition: transform 0.2s ease;
        }

        .name-item:hover {
            transform: translateX(-5px);
        }

        .name-item.present {
            border-color: #38ef7d;
        }

        .name-item.absent {
            border-color: #f45c43;
        }

        .history-section {
            animation: fadeInUp 0.8s ease 0.4s both;
            margin-top: 60px;
        }

        .history-title {
            color: white;
            font-size: 1.5em;
            margin-bottom: 20px;
            text-align: center;
        }

        .history-grid {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
            gap: 15px;
        }

        .history-btn {
            background: white;
            border: none;
            padding: 20px;
            border-radius: 12px;
            cursor: pointer;
            transition: all 0.3s ease;
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
            position: relative;
            overflow: hidden;
        }

        .history-btn::before {
            content: '';
            position: absolute;
            top: 0;
            left: -100%;
            width: 100%;
            height: 100%;
            background: linear-gradient(90deg, transparent, rgba(255, 255, 255, 0.3), transparent);
            transition: left 0.5s ease;
        }

        .history-btn:hover::before {
            left: 100%;
        }

        .history-btn:hover {
            transform: translateY(-3px);
            box-shadow: 0 8px 20px rgba(0, 0, 0, 0.3);
        }

        .history-date {
            font-weight: bold;
            color: #344b9d;
            font-size: 1.1em;
            margin-bottom: 5px;
        }

        .history-stats {
            font-size: 0.9em;
            color: #666;
        }

        .snapshot-action {
            margin: 34px;
            text-align: center;
            animation: fadeInUp 0.8s ease 0.4s both;
        }

        .save-snapshot-btn {
            background: linear-gradient(135deg, #4f46e5, #6366f1);
            color: #fff;
            border: none;
            border-radius: 14px;
            padding: 14px 28px;
            font-size: 18px;
            font-weight: 600;
            cursor: pointer;
            box-shadow: 0 10px 25px rgba(79, 70, 229, 0.35);
            transition: all 0.25s ease;
            letter-spacing: 0.3px;
        }

        .save-snapshot-btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 14px 35px rgba(79, 70, 229, 0.45);
        }

        .save-snapshot-btn:active {
            transform: translateY(0);
            box-shadow: 0 8px 20px rgba(79, 70, 229, 0.35);
        }

        .save-snapshot-btn:disabled {
            opacity: 0.6;
            cursor: not-allowed;
            box-shadow: none;
        }

        .snapshot-viewer {
            margin-top: 30px;
        }

        .history-btn.is-active {
            background: linear-gradient(135deg, #1f2933, #111827);
            /* غامق أنيق */
            border: 1px solid rgba(99, 102, 241, 0.6);
            /* بنفسجي هادئ */
            outline: none;
            box-shadow:
                0 12px 28px rgba(0, 0, 0, 0.35),
                inset 0 0 0 1px rgba(99, 102, 241, 0.25);
            transform: translateY(-2px);
        }

        .history-btn.is-active .history-date {
            color: #e5e7eb;
            font-weight: 600;
        }

        .history-btn.is-active .history-stats {
            color: #a5b4fc;
        }

        @keyframes fadeInDown {
            from {
                opacity: 0;
                transform: translateY(-30px);
            }

            to {
                opacity: 1;
                transform: translateY(0);
            }
        }

        @keyframes fadeInUp {
            from {
                opacity: 0;
                transform: translateY(30px);
            }

            to {
                opacity: 1;
                transform: translateY(0);
            }
        }

        @media (max-width: 768px) {
            .stats-container {
                grid-template-columns: 1fr;
            }

            .logo {
                font-size: 2em;
            }

            .history-grid {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>

<body>
    <div class="container">
        <!-- Header -->
        <div class="header">
            <div class="logo">MeshLink</div>
            <div class="subtitle" id="live-status">
                Last updated: --
            </div>
        </div>

        <!-- Stats Section -->
        <div class="stats-container">
            <!-- Present Card -->
            <div class="stat-card">
                <div class="stat-header">
                    <div class="stat-icon present" id="present-count">0</div>
                </div>
                <div class="stat-label">Present</div>
                <div class="names-list">
                    <div class="names-item present" id="present-list"></div>
                </div>
            </div>

            <!-- Absent Card -->
            <div class="stat-card">
                <div class="stat-header">
                    <div class="stat-icon absent" id="absent-count">0</div>
                </div>
                <div class="stat-label">Absent</div>
                <div class="names-list">
                    <div class="names-item absent" id="absent-list"></div>
                </div>
            </div>
        </div>

        <div class="snapshot-action">
            <form method="POST" action="{{ route('attendance-snapshots.store') }}" id="snapshot-form">
                @csrf
                <input type="hidden" name="present_ids" id="present-ids">
                <input type="hidden" name="absent_ids" id="absent-ids">

                <button type="submit" class="save-snapshot-btn">
                    💾 Save Attendance Snapshot
                </button>
            </form>
        </div>

        <!-- History Section -->
        <div class="history-section">
            <h2 class="history-title">Previous Attendance Records</h2>
            <div class="history-grid">
                @foreach ($snapshots as $snap)
                @php
                $presentCount = is_array($snap->present_ids) ? count($snap->present_ids) : 0;
                $absentCount = is_array($snap->absent_ids) ? count($snap->absent_ids) : 0;
                @endphp

                <button class="history-btn js-snapshot-btn" type="button" data-id="{{ $snap->id }}">
                    <div class="history-date">{{ $snap->created_at->format('F d, Y | H:i') }}</div>
                    <div class="history-stats">Present: {{ $presentCount }} | Absent: {{ $absentCount }}</div>
                </button>

                @endforeach
                <!-- <button class="history-btn">
                    <div class="history-date">January 10, 2026</div>
                    <div class="history-stats">Present: 18 | Absent: 2</div>
                </button>
                <button class="history-btn">
                    <div class="history-date">January 9, 2026</div>
                    <div class="history-stats">Present: 17 | Absent: 3</div>
                </button>
                <button class="history-btn">
                    <div class="history-date">January 8, 2026</div>
                    <div class="history-stats">Present: 16 | Absent: 4</div>
                </button>
                <button class="history-btn">
                    <div class="history-date">January 7, 2026</div>
                    <div class="history-stats">Present: 19 | Absent: 1</div>
                </button>
                <button class="history-btn">
                    <div class="history-date">January 6, 2026</div>
                    <div class="history-stats">Present: 15 | Absent: 5</div>
                </button>
                <button class="history-btn">
                    <div class="history-date">January 5, 2026</div>
                    <div class="history-stats">Present: 20 | Absent: 0</div>
                </button> -->
            </div>
        </div>

        <!-- Snapshot Viewer Section (hidden by default) -->
        <div class="snapshot-viewer" id="snapshot-viewer" style="display:none;">
            <h2 class="history-title" id="snapshot-viewer-title">Snapshot Details</h2>

            <div class="stats-container">
                <!-- Present Card -->
                <div class="stat-card">
                    <div class="stat-header">
                        <div class="stat-icon present" id="snapshot-present-count">0</div>
                    </div>
                    <div class="stat-label">Present</div>
                    <div class="names-list">
                        <div class="names-item present" id="snapshot-present-list"></div>
                    </div>
                </div>

                <!-- Absent Card -->
                <div class="stat-card">
                    <div class="stat-header">
                        <div class="stat-icon absent" id="snapshot-absent-count">0</div>
                    </div>
                    <div class="stat-label">Absent</div>
                    <div class="names-list">
                        <div class="names-item absent" id="snapshot-absent-list"></div>
                    </div>
                </div>
            </div>
        </div>

    </div>
    <script>
        const LIVE_THRESHOLD_MS = 3000;

        function formatDateLocal(isoString) {
            if (!isoString) return '--';
            const d = new Date(isoString);
            const pad = (n) => String(n).padStart(2, '0');
            return `${d.getFullYear()}-${pad(d.getMonth() + 1)}-${pad(d.getDate())}, ` +
                `${pad(d.getHours())}:${pad(d.getMinutes())}:${pad(d.getSeconds())}`;
        }

        function collectIds(containerId) {
            const container = document.getElementById(containerId);
            return Array.from(container.querySelectorAll('[data-id]'))
                .map(el => Number(el.dataset.id))
                .filter(n => Number.isFinite(n));
        }

        async function loadRollCall() {
            try {
                const res = await fetch('/api/rollcall/status');
                const data = await res.json();

                document.getElementById('present-count').innerText = data.present_count ?? 0;
                document.getElementById('absent-count').innerText = data.absent_count ?? 0;

                const presentList = document.getElementById('present-list');
                const absentList = document.getElementById('absent-list');

                presentList.innerHTML = '';
                absentList.innerHTML = '';

                (data.present || []).forEach(m => {
                    // مهم: m.id جاي من الكنترولر
                    presentList.insertAdjacentHTML('beforeend',
                        `<div class="name-item present" data-id="${m.id}">${m.name}</div>`
                    );
                });

                (data.absent || []).forEach(m => {
                    absentList.insertAdjacentHTML('beforeend',
                        `<div class="name-item absent" data-id="${m.id}">${m.name}</div>`
                    );
                });

                const el = document.getElementById('live-status');
                const lastSeen = data.last_seen;

                if (lastSeen) {
                    const diff = Date.now() - new Date(lastSeen).getTime();
                    el.innerText = (diff <= LIVE_THRESHOLD_MS) ? 'LIVE' : ('Last updated: ' + formatDateLocal(lastSeen));
                } else {
                    el.innerText = 'Last updated: --';
                }

            } catch (e) {
                document.getElementById('live-status').innerText = 'Last updated: -- (offline)';
            }
        }

        // ربط زر الحفظ بالفورم
        document.addEventListener('DOMContentLoaded', () => {
            loadRollCall();
            setInterval(loadRollCall, 2000);

            const form = document.getElementById('snapshot-form');
            if (form) {
                form.addEventListener('submit', (e) => {
                    const presentIds = collectIds('present-list');
                    const absentIds = collectIds('absent-list');

                    document.getElementById('present-ids').value = JSON.stringify(presentIds);
                    document.getElementById('absent-ids').value = JSON.stringify(absentIds);

                    if (presentIds.length === 0 && absentIds.length === 0) {
                        e.preventDefault();
                        alert('No attendance data to save yet.');
                    }
                });
            }
        });
    </script>

    <script>
        function formatNice(iso) {
            if (!iso) return '';
            const d = new Date(iso);
            const pad = n => String(n).padStart(2, '0');
            return `${d.getFullYear()}-${pad(d.getMonth()+1)}-${pad(d.getDate())} ${pad(d.getHours())}:${pad(d.getMinutes())}`;
        }

        async function loadSnapshot(snapshotId) {
            const res = await fetch(`/attendance-snapshots/${snapshotId}`, {
                headers: {
                    'Accept': 'application/json'
                }
            });
            if (!res.ok) throw new Error('Failed to fetch snapshot');
            return await res.json();
        }

        function renderSnapshot(data) {
            // show section
            const viewer = document.getElementById('snapshot-viewer');
            viewer.style.display = 'block';

            // title
            document.getElementById('snapshot-viewer-title').innerText =
                `Snapshot Details | ${formatNice(data.created_at)}`;

            // counts
            document.getElementById('snapshot-present-count').innerText = data.present_count ?? 0;
            document.getElementById('snapshot-absent-count').innerText = data.absent_count ?? 0;

            // lists
            const pList = document.getElementById('snapshot-present-list');
            const aList = document.getElementById('snapshot-absent-list');

            pList.innerHTML = '';
            aList.innerHTML = '';

            (data.present || []).forEach(m => {
                pList.innerHTML += `<div class="name-item present">${m.name}</div>`;
            });

            (data.absent || []).forEach(m => {
                aList.innerHTML += `<div class="name-item absent">${m.name}</div>`;
            });

            // scroll to viewer (اختياري)
            viewer.scrollIntoView({
                behavior: 'smooth',
                block: 'start'
            });
        }

        document.addEventListener('DOMContentLoaded', () => {
            document.querySelectorAll('.js-snapshot-btn').forEach(btn => {
                btn.addEventListener('click', async () => {
                    const id = btn.dataset.id;

                    // highlight active button (اختياري)
                    document.querySelectorAll('.js-snapshot-btn').forEach(b => b.classList.remove('is-active'));
                    btn.classList.add('is-active');

                    // loading state
                    const viewer = document.getElementById('snapshot-viewer');
                    viewer.style.display = 'block';
                    document.getElementById('snapshot-viewer-title').innerText = 'Snapshot Details | Loading...';
                    document.getElementById('snapshot-present-count').innerText = '...';
                    document.getElementById('snapshot-absent-count').innerText = '...';
                    document.getElementById('snapshot-present-list').innerHTML = '';
                    document.getElementById('snapshot-absent-list').innerHTML = '';

                    try {
                        const data = await loadSnapshot(id);
                        renderSnapshot(data);
                    } catch (e) {
                        document.getElementById('snapshot-viewer-title').innerText = 'Snapshot Details | Failed to load';
                    }
                });
            });
        });
    </script>

</body>

</html>