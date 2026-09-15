<?php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;

return new class extends Migration
{
    public function up(): void
    {
        Schema::create('rollcall_states', function (Blueprint $table) {
            $table->id(); // هنثبت دايمًا على id=1
            $table->json('present_codes')->nullable(); // ["M01","M02"]
            $table->timestamp('last_seen_at')->nullable(); // آخر مرة وصلنا JSON من ESP
            $table->timestamps();
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('rollcall_states');
    }
};
