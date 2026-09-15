<?php

namespace App\Http\Controllers;

use App\Http\Controllers\Controller;
use Illuminate\Http\Request;
use App\Models\AttendanceSnapshot;

class DashboardController extends Controller
{
    public function index()
    {
        $snapshots = AttendanceSnapshot::orderByDesc('created_at')->get();
        return view('dashboard', compact('snapshots'));
    }
}

