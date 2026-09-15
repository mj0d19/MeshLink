<?php

namespace Database\Seeders;

use Illuminate\Database\Seeder;
use App\Models\AttendanceSnapshot;
use Carbon\Carbon;

class AttendanceSnapshotSeeder extends Seeder
{
    public function run(): void
    {
        $data = [
            ['present' => range(1, 9), 'absent' => [10], 'dt' => '2026-01-10 10:30:00'],
            ['present' => range(1, 7), 'absent' => [8, 9, 10], 'dt' => '2026-01-09 10:30:00'],
            ['present' => range(1, 6), 'absent' => [7, 8, 9, 10], 'dt' => '2026-01-08 10:30:00'],
            ['present' => [1,2,3,4,5,6,7,8,9], 'absent' => [10], 'dt' => '2026-01-07 10:30:00'],
            ['present' => range(1, 5), 'absent' => [6,7,8,9,10], 'dt' => '2026-01-06 10:30:00'],
            ['present' => range(1, 10), 'absent' => [], 'dt' => '2026-01-05 10:30:00'],
        ];

        foreach ($data as $row) {
            AttendanceSnapshot::create([
                'present_ids' => $row['present'],
                'absent_ids'  => $row['absent'],
                'created_at'  => Carbon::parse($row['dt']),
                'updated_at'  => Carbon::parse($row['dt']),
            ]);
        }
    }
}
