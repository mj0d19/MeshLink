<?php

namespace Database\Seeders;

use Illuminate\Database\Seeder;
use App\Models\RollcallState;

class RollcallStateSeeder extends Seeder
{
    public function run(): void
    {
        RollcallState::updateOrCreate(
            ['id' => 1],
            [
                'present_codes' => ['M01', 'M02', 'M04', 'M05', 'M07', 'M09'],
                'last_seen_at' => now(),
            ]
        );
    }
}