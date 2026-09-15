<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use App\Models\AttendanceSnapshot;
use App\Models\Member;

class AttendanceSnapshotController extends Controller
{
    public function store(Request $request)
    {
        $request->validate([
            'present_ids' => ['required', 'string'],
            'absent_ids'  => ['required', 'string'],
        ]);

        $presentIds = json_decode($request->present_ids, true) ?? [];
        $absentIds  = json_decode($request->absent_ids, true) ?? [];

        AttendanceSnapshot::create([
            'present_ids' => $presentIds,
            'absent_ids'  => $absentIds,
        ]);

        return redirect()->back()->with('success', 'Attendance snapshot saved!');
    }

    public function show(AttendanceSnapshot $snapshot)
    {
        $presentIds = is_array($snapshot->present_ids) ? $snapshot->present_ids : [];
        $absentIds  = is_array($snapshot->absent_ids) ? $snapshot->absent_ids : [];

        $present = Member::whereIn('id', $presentIds)
            ->orderBy('member_code')
            ->get(['id', 'member_code', 'name']);

        $absent = Member::whereIn('id', $absentIds)
            ->orderBy('member_code')
            ->get(['id', 'member_code', 'name']);

        return response()->json([
            'id' => $snapshot->id,
            'created_at' => optional($snapshot->created_at)->toIso8601String(),
            'present_count' => $present->count(),
            'absent_count' => $absent->count(),
            'present' => $present,
            'absent' => $absent,
        ]);
    }
}
