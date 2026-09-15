<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use Illuminate\Http\Request;
use App\Models\Member;
use App\Models\RollcallState;

class RollCallController extends Controller
{
    // ✅ ESP يرسل present codes هنا
    // POST /api/rollcall/store
    public function store(Request $request)
    {
        $data = $request->validate([
            'present' => 'required|array',
            'present.*' => 'string',
        ]);

        $presentCodes = collect($data['present'])
            ->map(fn($c) => strtoupper(trim($c)))
            ->unique()
            ->values()
            ->all();

        RollcallState::updateOrCreate(
            ['id' => 1],
            [
                'present_codes' => $presentCodes,
                'last_seen_at' => now(),
            ]
        );

        return response()->json([
            'status' => 'ok',
            'saved_present_count' => count($presentCodes),
        ]);
    }

    public function status(Request $request)
    {
        $state = RollcallState::find(1);

        $presentCodes = [];
        $lastSeen = null;

        if ($state) {
            $presentCodes = $state->present_codes ?? [];
            $lastSeen = $state->last_seen_at ? $state->last_seen_at->toIso8601String() : null;
        }

        $presentMembers = Member::whereIn('member_code', $presentCodes)
            ->orderBy('member_code')
            ->get(['id', 'member_code', 'name']);

        $absentMembers = Member::whereNotIn('member_code', $presentCodes)
            ->orderBy('member_code')
            ->get(['id', 'member_code', 'name']);

        return response()->json([
            'present_count' => $presentMembers->count(),
            'absent_count'  => $absentMembers->count(),
            'present'       => $presentMembers,
            'absent'        => $absentMembers,
            'last_seen'     => $lastSeen,
        ]);
    }
}
